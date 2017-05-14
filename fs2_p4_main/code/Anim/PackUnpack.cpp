/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#ifndef UNITY_BUILD
#include "GrInternal.h"
#include "BmpMan.h"
#include "Palman.h"
#include "2d.h"
#include "AnimPlay.h"
#include "PackUnPack.h"
#include "AnimPlay.h"
#endif

int packer_code = PACKER_CODE;
int transparent_code = 254;

void anim_check_for_palette_change(anim_instance *instance) {
	if ( instance->parent->screen_sig != gr_screen.signature ) {
		instance->parent->screen_sig = gr_screen.signature;
	}
}

anim_instance *init_anim_instance(anim *ptr, int bpp)
{
	Assert(bpp != 16);
	anim_instance *inst;

	if (!ptr) {
		Int3();
		return NULL;
	}

	if ( ptr->flags & ANF_STREAMED ) {
		if ( ptr->file_offset < 0 ) {
			Int3();
			return NULL;
		}
	} else {
		if ( !ptr->data ) {
			Int3();
			return NULL;
		}
	}

	ptr->instance_count++;
	inst = (anim_instance *) malloc(sizeof(anim_instance));
	Assert(inst);
	inst->frame_num = -1;
	inst->last_frame_num = -1;
	inst->parent = ptr;
	inst->data = ptr->data;
	inst->file_offset = ptr->file_offset;
	inst->stop_now = FALSE;
	inst->aa_color = NULL;

	inst->frame = (ubyte *) malloc(inst->parent->width * inst->parent->height * bpp / 8);
	return inst;
}

void free_anim_instance(anim_instance *inst)
{
	Assert(inst->frame);
	free(inst->frame);
	inst->frame = NULL;
	inst->parent->instance_count--;	
	inst->parent = NULL;
	inst->data = NULL;
	inst->file_offset = -1;

	free(inst);	
}

int anim_get_next_frame(anim_instance *inst)
{
	int bm, bitmap_flags;
	int aabitmap = 0;
	int bpp = 16;

	if ( anim_instance_is_streamed(inst) ) {
		if ( inst->file_offset <= 0 ) {
			return -1;
		}
	} else {
		if (!inst->data)
			return -1;
	}

	inst->frame_num++;
	if (inst->frame_num >= inst->parent->total_frames) {
		inst->data = NULL;
		inst->file_offset = inst->parent->file_offset;
		return -1;
	}

	bitmap_flags = 0;

	bpp = kDefaultBpp;
	if(inst->aa_color != NULL){
		bitmap_flags |= BMP_AABITMAP;
		aabitmap = 1;
		bpp = 8;
	}

	anim_check_for_palette_change(inst);

	BM_SELECT_TEX_FORMAT();

	if ( anim_instance_is_streamed(inst) ) {
		inst->file_offset = unpack_frame_from_file(inst, inst->frame, inst->parent->width*inst->parent->height, aabitmap, bpp);
	} else {
		inst->data = unpack_frame(inst, inst->data, inst->frame, inst->parent->width*inst->parent->height, aabitmap, bpp);
	}

	bm = bm_create(bpp, inst->parent->width, inst->parent->height, inst->frame, bitmap_flags);
	bm_free_data_without_unlock(bm);
	return bm;
}

ubyte *anim_get_next_raw_buffer(anim_instance *inst, int aabitmap, int bpp)
{	
	if ( anim_instance_is_streamed(inst) ) {
		if ( inst->file_offset < 0 ) {
			return NULL;
		}
	} else {
		if (!inst->data){
			return NULL;
		}
	}

	inst->frame_num++;
	if (inst->frame_num >= inst->parent->total_frames) {
		inst->data = NULL;
		inst->file_offset = inst->parent->file_offset;
		return NULL;
	}

	anim_check_for_palette_change(inst);

	if ( anim_instance_is_streamed(inst) ) {
			inst->file_offset = unpack_frame_from_file(inst, inst->frame, inst->parent->width*inst->parent->height, aabitmap, bpp);
	} else {
			inst->data = unpack_frame(inst, inst->data, inst->frame, inst->parent->width*inst->parent->height, aabitmap, bpp);
	}

	return inst->frame;
}

// --------------------------------------------------------------------
// anim_get_frame()
//
// Get a bitmap id from the anim_instance for the specified frame_num
//
//	input:	*inst			=>		pointer to anim instance
//				frame_num	=>		frame number to get (first frame is 0)
//
int anim_get_frame(anim_instance *inst, int frame_num)
{
	int			bm, bitmap_flags, key = 0, offset = 0;
	int idx;
	int bpp = kDefaultBpp;
	int aabitmap = 0;

	if ((frame_num < 0) || (frame_num >= inst->parent->total_frames))  // illegal frame number
		return -1;

	int need_reset = 0;
	if ( anim_instance_is_streamed(inst) ) {
		if ( inst->file_offset < 0 ) {
			need_reset = 1;
		}
	} else {
		if ( !inst->data ) {
			need_reset = 1;
		}
	}

	if (need_reset || (inst->frame_num >= inst->parent->total_frames)) {  // reset to valid info
		inst->data = inst->parent->data;
		inst->file_offset = inst->parent->file_offset;
		inst->frame_num = 0;
	}

	bitmap_flags = 0;

	if ( inst->frame_num == frame_num ) {
		bm = bm_create(32, inst->parent->width, inst->parent->height, inst->frame, bitmap_flags);
		bm_free_data_without_unlock(bm);
		return bm;
	}

	idx = 0;
	key = 0;
	while(idx < inst->parent->num_keys){			
		if (( (inst->parent->keys[idx].frame_num-1) <= frame_num) && ( (inst->parent->keys[idx].frame_num-1) > key)) {  // find closest key
			key = inst->parent->keys[idx].frame_num - 1;
			offset = inst->parent->keys[idx].offset;
				
			if ( key == frame_num )
				break;
		}
		idx++;
	}
			
	if ( key == frame_num ) {
		inst->frame_num = key;

		if ( anim_instance_is_streamed(inst) ) {
			inst->file_offset = inst->parent->file_offset + offset;
		} else {
			inst->data = inst->parent->data + offset;
		}

		anim_check_for_palette_change(inst);

		if ( anim_instance_is_streamed(inst) ) {
				inst->file_offset = unpack_frame_from_file(inst, inst->frame, inst->parent->width*inst->parent->height, aabitmap, bpp);
		} else {
				inst->data = unpack_frame(inst, inst->data, inst->frame, inst->parent->width*inst->parent->height, aabitmap, bpp);
		}

		goto create_bitmap;
	}

	if (key > inst->frame_num)  // best key is closer than current position
	{
		inst->frame_num = key;

		if ( anim_instance_is_streamed(inst) ) {
			inst->file_offset = inst->parent->file_offset + offset;
		} else {
			inst->data = inst->parent->data + offset;
		}

		anim_check_for_palette_change(inst);

		if ( anim_instance_is_streamed(inst) ) {
			inst->file_offset = unpack_frame_from_file(inst, inst->frame, inst->parent->width*inst->parent->height, aabitmap, bpp);
		} else {
			inst->data = unpack_frame(inst, inst->data, inst->frame, inst->parent->width*inst->parent->height, aabitmap, bpp);
		}
	}

	while (inst->frame_num != frame_num) {
		anim_check_for_palette_change(inst);

		if ( anim_instance_is_streamed(inst) ) {
			inst->file_offset = unpack_frame_from_file(inst, inst->frame, inst->parent->width*inst->parent->height, aabitmap, bpp);
		} else {
			inst->data = unpack_frame(inst, inst->data, inst->frame, inst->parent->width*inst->parent->height, aabitmap, bpp);
		}
		inst->frame_num++;
	}

	create_bitmap:

	bm = bm_create(32, inst->parent->width, inst->parent->height, inst->frame, bitmap_flags);
	bm_free_data_without_unlock(bm);
	return bm;
}

// frame = frame pixel data to pack
// save = memory to store packed data to
// size = number of bytes to pack
// max = maximum number of packed bytes (size of buffer)
// returns: actual number of bytes data packed to or -1 if error
int pack_key_frame(ubyte *frame, ubyte *save, long size, long max, int compress_type)
{
	int last = -32768, count = 0;
	long packed_size = 1;

	switch ( compress_type ) {
		case PACKING_METHOD_RLE_KEY:
			*save++ = PACKING_METHOD_RLE_KEY;
			while (size--) {
				if (*frame != last || count > 255) {
					if (packed_size + 3 >= max)
						return -1;

					if (count < 3) {
						if (last == packer_code) {
							*save++ = (ubyte)packer_code;
							*save++ = (ubyte)(count - 1);
							packed_size += 2;

						} else
							while (count--) {
								*save++ = (ubyte)last;
								packed_size++;
							}

					} else {
						*save++ = (ubyte)packer_code;
						*save++ = (ubyte)(count - 1);
						*save++ = (ubyte)last;
						packed_size += 3;
					}

					count = 0;
					last = *frame;
				}

				count++;
				frame++;
			}

			if (packed_size + 3 >= max)
				return -1;

			if (count < 3) {
				if (last == packer_code) {
					*save++ = (ubyte)packer_code;
					*save++ = (ubyte)(count - 1);
					packed_size += 2;

				} else
					while (count--) {
						*save++ = (ubyte)last;
						packed_size++;
					}

			} else {
				*save++ = (ubyte)packer_code;
				*save++ = (ubyte)(count - 1);
				*save++ = (ubyte)last;
				packed_size += 3;
			}
			break;

		case PACKING_METHOD_STD_RLE_KEY: {
			ubyte *dest_start;
			int i;

			dest_start = save;
			count = 1;

			last = *frame++;
			*save++ = PACKING_METHOD_STD_RLE_KEY;
			for (i=1; i < size; i++ )	{

				if ( *frame != last ) {
					if ( count ) {

						if (packed_size + 2 >= max)
							return -1;

						if ( (count == 1) && !(last & STD_RLE_CODE) ) {
							*save++ = (ubyte)last;
							packed_size++;
							Assert( last != STD_RLE_CODE );
//							printf("Just packed %d 1 times, since pixel change, no count included\n",last);
						}
						else {
							count |= STD_RLE_CODE;
							*save++ = (ubyte)count;
							*save++ = (ubyte)last;
							packed_size += 2;
//							printf("Just packed %d %d times, since pixel change\n",last,count);
						}
					}
		
					last = *frame;
					count = 0;
				}

				count++;
				frame++;

				if ( count == 127 ) {
					count |= STD_RLE_CODE;
					*save++ = (ubyte)count;
					*save++ = (ubyte)last;
					packed_size += 2;
					count = 0;
//					printf("Just packed %d %d times, since count overflow\n",last,count);

				}
			}	// end for

			if (count)	{

				if (packed_size + 2 >= max)
					return -1;

				if ( (count == 1) && !(last & STD_RLE_CODE) ) {
					*save++ = (ubyte)last;
					packed_size++;
//					printf("Just packed %d 1 times, at end since single pixel, no count\n",last);
					Assert( last != STD_RLE_CODE );
				}
				else {
					count |= STD_RLE_CODE;
					*save++ = (ubyte)count;
					*save++ = (ubyte)last;
					packed_size += 2;
//					printf("Just packed %d %d times, at end since pixel change\n",last,count);
				}
			}

			Assert(packed_size == (save-dest_start) );
			return packed_size;
			break;
			}

		default:
			Assert(0);
			return -1;
			break;
	} // end switch

	return packed_size;
}

// frame = frame pixel data to pack
// frame2 = previous frame's pixel data
// save = memory to store packed data to
// size = number of bytes to pack
// max = maximum number of packed bytes (size of buffer)
// returns: actual number of bytes data packed to or -1 if error
int pack_frame(ubyte *frame, ubyte *frame2, ubyte *save, long size, long max, int compress_type)
{
	int pixel, last = -32768, count = 0, i;
	long packed_size = 1;

	switch ( compress_type ) {
		case PACKING_METHOD_RLE:					// Hoffoss RLE regular frame
			*save++ = PACKING_METHOD_RLE;
			while (size--) {
				if (*frame != *frame2++)
					pixel = *frame;
				else
					pixel = transparent_code;

				if (pixel != last || count > 255) {
					if (packed_size + 3 >= max)
						return -1;

					if (count < 3) {
						if (last == packer_code) {
							*save++ = (ubyte)packer_code;
							*save++ = (ubyte)(count - 1);
							packed_size += 2;

						} else
							while (count--) {
								*save++ = (ubyte)last;
								packed_size++;
							}

					} else {
						*save++ = (ubyte)packer_code;
						*save++ = (ubyte)(count - 1);
						*save++ = (ubyte)last;
						packed_size += 3;
					}

					count = 0;
					last = pixel;
				}

				frame++;
				count++;
			}

			if (packed_size + 3 >= max)
				return -1;

			if (count < 3) {
				if (last == packer_code) {
					*save++ = (ubyte)packer_code;
					*save++ = (ubyte)(count - 1);
					packed_size += 2;

				} else
					while (count--) {
						*save++ = (ubyte)last;
						packed_size++;
					}

			} else {
				*save++ = (ubyte)(packer_code);
				*save++ = (ubyte)(count - 1);
				*save++ = (ubyte)(last);
				packed_size += 3;
			}
			break;

		case PACKING_METHOD_STD_RLE: {		// high bit count regular RLE frame

			ubyte *dest_start;

			dest_start = save;
			count = 1;

			if (*frame++ != *frame2++)
				last = *frame;
			else
				last = transparent_code;

			*save++ = PACKING_METHOD_STD_RLE;
			for (i=1; i < size; i++ )	{

				if (*frame != *frame2++)
					pixel = *frame;
				else
					pixel = transparent_code;

				if ( pixel != last ) {
					if ( count ) {

						if (packed_size + 2 >= max)
							return -1;

						if ( (count == 1) && !(last & STD_RLE_CODE) ) {
							*save++ = (ubyte)last;
							packed_size++;
							Assert( last != STD_RLE_CODE );
						}
						else {
							count |= STD_RLE_CODE;
							*save++ = (ubyte)count;
							*save++ = (ubyte)last;
							packed_size += 2;
						}
					}
		
					last = pixel;
					count = 0;
				}

				count++;
				frame++;

				if ( count == 127 ) {
					count |= STD_RLE_CODE;
					*save++ = (ubyte)count;
					*save++ = (ubyte)last;
					packed_size += 2;
					count = 0;
				}
			}	// end for

			if (count)	{

				if (packed_size + 2 >= max)
					return -1;

				if ( (count == 1) && !(last & STD_RLE_CODE) ) {
					*save++ = (ubyte)last;
					packed_size++;
					Assert( last != STD_RLE_CODE );
				}
				else {
					count |= STD_RLE_CODE;
					*save++ = (ubyte)count;
					*save++ = (ubyte)last;
					packed_size += 2;
				}
			}

			Assert(packed_size == (save-dest_start) );
			return packed_size;
			break;
			}

		default:
			Assert(0);
			return -1;
			break;
	} // end switch

	return packed_size;
}

// unpack a pixel given the passed index and the anim_instance's palette, return bytes stuffed
int unpack_pixel(anim_instance *ai, ubyte *data, ubyte pix, int aabitmap, int bpp)
{
	int bit_24;
	int bit_32 = 0;
	ushort bit_16 = 0;	
	ubyte bit_8 = 0;
	ubyte al = 0;
	ubyte r, g, b;
	anim *a = ai->parent;
	Assert(a);	

	// if this is an aabitmap, don't run through the palette
	if(aabitmap){
		switch(bpp){
		case 32 :
			bit_32 = pix;
			break;
		case 16 : 
			bit_16 = (ushort)pix;
			break;
		case 8:
			bit_8 = pix;
			break;
		default:
			Int3();
		}
	} else {

		// if the pixel value is 255, or is the xparent color, make it so		
		if(((a->palette[pix*3] == a->xparent_r) && (a->palette[pix*3+1] == a->xparent_g) && (a->palette[pix*3+2] == a->xparent_b)) ){
			r = b = 0;
			g = 255;
			if(bpp == 32)
			{
				bit_32 = 0;
			}
			else
			{
				bm_set_components((ubyte*)&bit_16, &r, &g, &b, &al);
			}
		} else {
			// stuff the 24 bit value
			

			if(bpp == 32)
			{
				//memcpy(&bit_32, &ai->parent->palette[pix * 3], 3);
				((ubyte*) &bit_32)[BM_32_B] = ai->parent->palette[pix * 3 + 2];
				((ubyte*) &bit_32)[BM_32_G] = ai->parent->palette[pix * 3 + 1];
				((ubyte*) &bit_32)[BM_32_R] = ai->parent->palette[pix * 3 + 0];
				((ubyte*) &bit_32)[BM_32_A] = 255;
			}
			else
			{
				// convert to 16 bit
				memcpy(&bit_24, &ai->parent->palette[pix * 3], 3);
				bm_24_to_16(bit_24, &bit_16);
			}
		}
	}

	// stuff the pixel
	switch(bpp){
	case 32 :
		memcpy(data, &bit_32, sizeof(int));
		return sizeof(int); // size of data passed in source
	case 16 :
		memcpy(data, &bit_16, sizeof(ushort));
		return sizeof(ushort);

	case 8 :
		*data = bit_8;		
		return sizeof(ubyte);	
	}

	Int3();
	return 0;
}

// unpack a pixel given the passed index and the anim_instance's palette, return bytes stuffed
int unpack_pixel_count(anim_instance *ai, ubyte *data, ubyte pix, int count, int aabitmap, int bpp)
{
	int bit_24 = -1;
	int idx;
	ubyte al = 0;
	int bit_32 = 0;
	ushort bit_16 = 0;	
	ubyte bit_8 = 0;
	anim *a = ai->parent;
	ubyte r, g, b;
	Assert(a);	

	// if this is an aabitmap, don't run through the palette
	if(aabitmap){
		switch(bpp){
		case 16 : 
			bit_16 = (ushort)pix;
			break;
		case 8 :
			bit_8 = pix;
			break;
		default :
			Int3();			
		}
	} else {		
		// if the pixel value is 255, or is the xparent color, make it so		
		if(((a->palette[pix*3] == a->xparent_r) && (a->palette[pix*3+1] == a->xparent_g) && (a->palette[pix*3+2] == a->xparent_b)) ){
			if(bpp == 32)
			{
				((ubyte *) &bit_32)[BM_32_R] = 0;
				((ubyte *) &bit_32)[BM_32_G] = 0;
				((ubyte *) &bit_32)[BM_32_B] = 0;
				((ubyte *) &bit_32)[BM_32_A] = 0;
			}
			else
			{
				r = b = 0;
				g = 255;
				bm_set_components((ubyte*)&bit_16, &r, &g, &b, &al);
			}
		} else {

			if(bpp == 32)
			{
				((ubyte *) &bit_32)[BM_32_B] = ai->parent->palette[pix * 3 + 2];
				((ubyte *) &bit_32)[BM_32_G] = ai->parent->palette[pix * 3 + 1];
				((ubyte *) &bit_32)[BM_32_R] = ai->parent->palette[pix * 3 + 0];
				((ubyte *) &bit_32)[BM_32_A] = 255;
			}
			else
			{
				// stuff the 24 bit value
				memcpy(&bit_24, &ai->parent->palette[pix * 3], 3);

				// convert to 16 bit
				bm_24_to_16(bit_24, &bit_16);
			}
		}
	}
	
	// stuff the pixel
	for(idx=0; idx<count; idx++){
		switch(bpp){
		case 32 :
			memcpy(data + (idx*4), &bit_32, sizeof(int));
			break;
		case 16 :
			memcpy(data + (idx*2), &bit_16, sizeof(ushort));
			break;
		case 8 :
			*(data + idx) = bit_8;
			break;
		}
	}

	return bpp / 8 * count;
}

// ptr = packed data to unpack
// frame = where to store unpacked data to
// size = total number of unpacked pixels requested
ubyte	*unpack_frame(anim_instance *ai, ubyte *ptr, ubyte *frame, int size, int aabitmap, int bpp)
{
	int	value, count = 0;
	int stuffed;			
	int pixel_size = bpp / 8;
	Assert(bpp != 16);

	if (*ptr == PACKING_METHOD_RLE_KEY) {  // key frame, Hoffoss's RLE format
		ptr++;
		while (size > 0) {
			value = *ptr++;
			if (value != packer_code) {

				stuffed = unpack_pixel(ai, frame, (ubyte)value, aabitmap, bpp);

				frame += stuffed;
				size--;
			} else {
				count = *ptr++;
				if (count < 2){
					value = packer_code;
				} else {
					value = *ptr++;
				}

				if (++count > size){
					count = size;
				}
					
				stuffed = unpack_pixel_count(ai, frame, (ubyte)value, count, aabitmap, bpp);

				frame += stuffed;
				size -= count;
			}
		}
	}
	else if ( *ptr == PACKING_METHOD_STD_RLE_KEY) {	// key frame, with high bit as count
		ptr++;
		while (size > 0) {
			value = *ptr++;
			if ( !(value & STD_RLE_CODE) ) {

				stuffed = unpack_pixel(ai, frame, (ubyte)value, aabitmap, bpp);

				frame += stuffed;
				size--;
			} else {
				count = value & (~STD_RLE_CODE);
				value = *ptr++;

				size -= count;
				Assert(size >= 0);

				stuffed = unpack_pixel_count(ai, frame, (ubyte)value, count, aabitmap, bpp);

				frame += stuffed;
			}
		}
	}
	else if (*ptr == PACKING_METHOD_RLE) {  // normal frame, Hoffoss's RLE format

// test code, to show unused pixels
// memset(frame, 255, size);
	
		ptr++;
		while (size > 0) {
			value = *ptr++;
			if (value != packer_code) {
				if (value != transparent_code) {
					stuffed = unpack_pixel(ai, frame, (ubyte)value, aabitmap, bpp);
				} else {
					// temporary pixel
					stuffed = pixel_size;
				}

				frame += stuffed;
				size--;
			} else {
				count = *ptr++;
				if (count < 2){
					value = packer_code;
				} else {
					value = *ptr++;
				}

				if (++count > size){
					count = size;
				}

				size -= count;
				Assert(size >= 0);

				if (value != transparent_code ) {
					stuffed = unpack_pixel_count(ai, frame, (ubyte)value, count, aabitmap, bpp);
				} else {
					stuffed = count * pixel_size;
				}

				frame += stuffed;
			}
		}

	}
	else if ( *ptr == PACKING_METHOD_STD_RLE) {	// normal frame, with high bit as count
		ptr++;
		while (size > 0) {
			value = *ptr++;
			if ( !(value & STD_RLE_CODE) ) {
				if (value != transparent_code) {
					stuffed = unpack_pixel(ai, frame, (ubyte)value, aabitmap, bpp);
				} else {
					stuffed = pixel_size;
				}

				frame += stuffed;
				size--;
			} else {
				count = value & (~STD_RLE_CODE);
				value = *ptr++;

				size -= count;
				Assert(size >= 0);

				if (value != transparent_code) {
					stuffed = unpack_pixel_count(ai, frame, (ubyte)value, count, aabitmap, bpp);
				} else {					
					stuffed = pixel_size * count;
				}

				frame += stuffed;
			}
		}
	}
	else {
		Assert(0);  // unknown packing method
	}

	return ptr;
}

// ptr = packed data to unpack
// frame = where to store unpacked data to
// size = total number of unpacked pixels requested
int unpack_frame_from_file(anim_instance *ai, ubyte *frame, int size, int aabitmap, int bpp)
{
	int	value, count = 0;
	int	offset = 0;
	int stuffed;	
	int pixel_size = bpp / 8;

	Assert(bpp != 16);
	if (anim_instance_get_byte(ai,offset) == PACKING_METHOD_RLE_KEY) {  // key frame, Hoffoss's RLE format
		offset++;
		while (size > 0) {
			value = anim_instance_get_byte(ai,offset);
			offset++;
			if (value != packer_code) {

				stuffed = unpack_pixel(ai, frame, (ubyte)value, aabitmap, bpp);

				frame += stuffed;
				size--;
			} else {
				count = anim_instance_get_byte(ai,offset);
				offset++;
				if (count < 2) {
					value = packer_code;
				} else {
					value = anim_instance_get_byte(ai,offset);
					offset++;
				}

				if (++count > size){
					count = size;
				}

				stuffed = unpack_pixel_count(ai, frame, (ubyte)value, count, aabitmap, bpp);

				frame += stuffed;
				size -= count;
			}
		}
	}
	else if ( anim_instance_get_byte(ai,offset) == PACKING_METHOD_STD_RLE_KEY) {	// key frame, with high bit as count
		offset++;
		while (size > 0) {
			value = anim_instance_get_byte(ai,offset);
			offset++;
			if ( !(value & STD_RLE_CODE) ) {

				stuffed = unpack_pixel(ai, frame, (ubyte)value, aabitmap, bpp);

				frame += stuffed;
				size--;
			} else {
				count = value & (~STD_RLE_CODE);
				value = anim_instance_get_byte(ai,offset);
				offset++;

				size -= count;
				Assert(size >= 0);

				stuffed = unpack_pixel_count(ai, frame, (ubyte)value, count, aabitmap, bpp);

				frame += stuffed;
			}
		}
	}
	else if (anim_instance_get_byte(ai,offset) == PACKING_METHOD_RLE) {  // normal frame, Hoffoss's RLE format

// test code, to show unused pixels
// memset(frame, 255, size);
	
		offset++;
		while (size > 0) {
			value = anim_instance_get_byte(ai,offset);
			offset++;
			if (value != packer_code) {
				if (value != transparent_code) {
					stuffed = unpack_pixel(ai, frame, (ubyte)value, aabitmap, bpp);
				} else {
					stuffed = pixel_size;
				}

				frame += stuffed;
				size--;
			} else {
				count = anim_instance_get_byte(ai,offset);
				offset++;

				if (count < 2) {
					value = packer_code;
				} else {
					value = anim_instance_get_byte(ai,offset);
					offset++;
				}
				if (++count > size){
					count = size;
				}

				size -= count;
				Assert(size >= 0);

				if (value != transparent_code ) {
					stuffed = unpack_pixel_count(ai, frame, (ubyte)value, count, aabitmap, bpp);
				} else {
					stuffed = pixel_size * count;
				}

				frame += stuffed;
			}
		}

	}
	else if ( anim_instance_get_byte(ai,offset) ) {	// normal frame, with high bit as count
		offset++;
		while (size > 0) {
			value = anim_instance_get_byte(ai,offset);
			offset++;
			if ( !(value & STD_RLE_CODE) ) {
				if (value != transparent_code) {
					stuffed = unpack_pixel(ai, frame, (ubyte)value, aabitmap, bpp);
				} else {
					stuffed = pixel_size;
				}

				frame += stuffed;
				size--;
			} else {
				count = value & (~STD_RLE_CODE);
				value = anim_instance_get_byte(ai,offset);
				offset++;

				size -= count;
				Assert(size >= 0);

				if (value != transparent_code) {
					stuffed = unpack_pixel_count(ai, frame, (ubyte)value, count, aabitmap, bpp);
				} else {					
					stuffed = pixel_size * count;
				}

				frame += stuffed;
			}
		}
	}
	else {
		Int3();  // unknown packing method
	}

	return ai->file_offset + offset;
}


bool AnimInstanceGroup::Load(const char * const filename)
{
	m_anim = anim_load(filename);
	Assert( m_anim );

	for(int i = 0; i < m_anim->total_frames; i++)
	{
		anim_instance *instance = init_anim_instance(m_anim, kDefaultBpp);
		Assert( instance  );
		int bitmapId = anim_get_frame(instance, i);

		m_instances.push_back(instance);
		m_bitmapIDs.push_back(bitmapId);
	}

	return m_anim != 0;
}

int AnimInstanceGroup::GetBitmap(int frame)
{
	return m_bitmapIDs[frame];
}
int AnimInstanceGroup::GetNumFrames() 
{
	return m_bitmapIDs.size();
}

void AnimInstanceGroup::Unload()
{
	for(uint i = 0; i < m_instances.size(); i++)
	{
		free_anim_instance(m_instances[i]);
	}

	anim_free(m_anim);
	m_anim = 0;

	m_instances.clear();
	m_bitmapIDs.clear();
}
