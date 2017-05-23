/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>

#include "scaler.h"
#include "2d.h"
#include "GrInternal.h"
#include "floating.h"
#include "bmpman.h"
#include "palman.h"
#include "TmapScanline.h"
#include "SystemVars.h"
#include "Key.h"
#include "Colors.h"

#define MIN_SCALE_FACTOR 0.0001f

#define USE_COMPILED_CODE

#define TRANSPARENCY_COLOR_8		0xff
#define TRANSPARENCY_COLOR_16		0xffff
#define TRANSPARENCY_COLOR_32		0xffffffff

#define FIND_SCALED_NUM(x,x0,x1,y0,y1) (((((x)-(x0))*((y1)-(y0)))/((x1)-(x0)))+(y0))

#define MAX_CODE_SIZE 32768		//65536 JAS: Determed to be 8208 on April1,98, 16K seems safe

ubyte compiled_code[MAX_CODE_SIZE];

static int Max_size = 0;

/*
void test_code()
{
	_asm mov ax, [esi+0xabcdef12]
	_asm cmp ax, 255
	_asm je  0xabcdef12
	_asm mov [edi+0xabcdef12], ax
	_asm mov ax, [esi+0xabcdef12]
}
*/



//----------------------------------------------------
// scaler_create_compiled_code8
//
// Creates code that looks like:
//
// @@: mov al, [esi+????]
//     cmp al, TRANSPARENCY_COLOR_8
//     je  @f   ; jump to next @@ label
//     mov [edi+???], al    ; If the source pixel is scaled up
//     mov [edi+???], al    ; there might be a lot of these lines
//     ...
// @@: mov al, [esi+????]
//

ubyte *scaler_create_compiled_code8( int w, fix u, fix du )
{
	int last_u, x;
	ubyte * cc;
	uint * last_jmp_pos;

	cc = compiled_code;

	//if ( abs(du) < F1_0 / 4 ) *cc++ = 0xCC;

//	*cc++ = 0xCC;	// Int3
//	*cc++ = 0xc3;	// RET

	last_u = -1;

	last_jmp_pos=NULL;

	for (x=0; x<w; x++ )			{
		if ( last_u != f2i(u) )	{
			if ( last_jmp_pos )	{
				*last_jmp_pos = (uint)cc - (uint)last_jmp_pos - 4;
			}
			*cc++ = 0x8a;	*cc++ = 0x86; // mov al, [esi+imm]
			*(uint *)cc = f2i(u); cc += 4;
			last_u = f2i(u);

			*cc++ = 0x3c; *cc++ = TRANSPARENCY_COLOR_8;	// cmp al, 255
			*cc++ = 0x0f; *cc++ = 0x84;   // je rel32
			last_jmp_pos = (uint *)cc;
			cc += 4;		
		}
		
	
		*cc++ = 0x88;	*cc++ = 0x87; // mov [edi+imm], al
		*(uint *)cc = x; cc += 4;

		u += du;
	}
	if ( last_jmp_pos )	{
		*last_jmp_pos = (uint)cc - (uint)last_jmp_pos - 4;
	}
	*cc++ = 0xc3;	// RET

	if ( cc >= &compiled_code[MAX_CODE_SIZE] )
		Int3();		// GET JOHN NOW!

#ifdef FIND_MAX_SIZE
	int size = cc - compiled_code;
	if ( size > Max_size )	{
		Max_size = size;
		mprintf(( "Max size = %d\n", size ));
	}
#endif

	return compiled_code;
}

ubyte *scaler_create_compiled_code8_stippled( int w, fix u, fix du )
{
	int last_u, x;
	ubyte * cc;
	uint * last_jmp_pos;

	cc = compiled_code;

	//if ( abs(du) < F1_0 / 4 ) *cc++ = 0xCC;

//	*cc++ = 0xCC;	// Int3
//	*cc++ = 0xc3;	// RET

	last_u = -1;

	last_jmp_pos=NULL;

	for (x=0; x<w-1; x+=2 )			{
		if ( last_u != f2i(u) )	{
			if ( last_jmp_pos )	{
				*last_jmp_pos = (uint)cc - (uint)last_jmp_pos - 4;
			}
			*cc++ = 0x8a;	*cc++ = 0x86; // mov al, [esi+imm]
			*(uint *)cc = f2i(u); cc += 4;
			last_u = f2i(u);

			*cc++ = 0x3c; *cc++ = TRANSPARENCY_COLOR_8;	// cmp al, 255
			*cc++ = 0x0f; *cc++ = 0x84;   // je rel32
			last_jmp_pos = (uint *)cc;
			cc += 4;		
		}
		
	
		*cc++ = 0x88;	*cc++ = 0x87; // mov [edi+imm], al
		*(uint *)cc = x; cc += 4;

		u += du*2;
	}
	if ( last_jmp_pos )	{
		*last_jmp_pos = (uint)cc - (uint)last_jmp_pos - 4;
	}
	*cc++ = 0xc3;	// RET

	if ( cc >= &compiled_code[MAX_CODE_SIZE] )
		Int3();		// GET JOHN NOW!

#ifdef FIND_MAX_SIZE
	int size = cc - compiled_code;
	if ( size > Max_size )	{
		Max_size = size;
		mprintf(( "Max size = %d\n", size ));
	}
#endif

	return compiled_code;
}

void test_code1()
{
	_asm mov ebx, -1
	_asm xor eax, eax
	_asm xor ebx, ebx
	_asm mov	bl, BYTE PTR [edi-1412567278]
	_asm add ebx, eax
	_asm mov ebx, [ecx+ebx]	; blend it
	_asm cmp ebp, [edx]
	_asm add edx, 4
	_asm jl [0xABCDEF12]
	
//     xor eax, eax			; avoid ppro partial register stall
//     mov ah, [esi+????]   ; get the foreground pixel
//     ; the following lines might be repeated
//     xor ebx, ebx			; avoid ppro partial register stall
//     mov bl, [edi+????]   ; get the background pixel
//     mov ebx, [ecx+ebx]	; blend it
//     mov [edi+????], bl   ; write it
}

/*
  00130	b8 00 00 00 00	mov	eax, 0
  00135	8a a6 12 ef cd ab		mov	ah, BYTE PTR [esi-1412567278]
  0013b	8a 87 12 ef cd ab		mov	al, BYTE PTR [edi-1412567278]
  00141	8a 1c 01	            mov	bl, BYTE PTR [ecx+eax]
  00141	8b 1c 01					mov	ebx, DWORD PTR [ecx+eax]
  00144	88 9f 12 ef cd ab		mov	BYTE PTR [edi-1412567278], bl


  00130	33 c0		xor	eax, eax
  00132	33 db		xor	ebx, ebx
  00134	8a 9f 12 ef cd	ab		mov	bl, BYTE PTR [edi-1412567278]
  0013a	03 d8		add	ebx, eax
  0013c	8b 1c 19	mov	ebx, DWORD PTR [ecx+ebx]

  0013f	3b 2a		cmp	ebp, DWORD PTR [edx]
  00141	83 c2 04	add	edx, 4


*/

//----------------------------------------------------
// scaler_create_compiled_code8_alpha
//
// Creates code that looks like:

//=============== Pentium ======================
// mov eax, 0
//     mov ah, [esi+????]   ; get the foreground pixel
//     ; the following lines might be repeated
//     mov al, [edi+????]   ; get the background pixel
//     mov bl, [ecx+eax]	; blend it
//     mov [edi+????], bl   ; write it
//     ...

//============= Pentium Pro code =============
//     xor eax, eax			; avoid ppro partial register stall
//     mov ah, [esi+????]   ; get the foreground pixel
//     ; the following lines might be repeated
//     xor ebx, ebx			; avoid ppro partial register stall
//     mov bl, [edi+????]   ; get the background pixel
//     mov ebx, [ecx+ebx]	; blend it
//     mov [edi+????], bl   ; write it


ubyte *scaler_create_compiled_code8_alpha( int w, fix u, fix du )
{
	int last_u, x;
	ubyte * cc;

	cc = compiled_code;

	//if ( abs(du) < F1_0 / 4 ) *cc++ = 0xCC;

	//*cc++ = 0xCC;	// Int3
	//*cc++ = 0xc3;	// RET

	last_u = -1;

	if ( Gr_cpu	> 5 )	{
		// Pentium Pro optimized code.

		for (x=0; x<w; x++ )			{
			if ( last_u != f2i(u) )	{
				*cc++ = 0x33;	*cc++ = 0xc0; // xor eax, eax
				*cc++ = 0x8a;	*cc++ = 0xa6; // mov ah, [esi+imm]
				//*cc++ = 0x8a;	*cc++ = 0x86; // mov al, [esi+imm]
				*(uint *)cc = f2i(u); cc += 4;
				last_u = f2i(u);
			}
			
  			*cc++ = 0x33;	*cc++ = 0xdb;		// xor ebx, ebx
			
  			*cc++ = 0x8a;	*cc++ = 0x9f; 
			*(uint *)cc = x; cc += 4;		// mov bl, [edi+imm]

  			*cc++ = 0x03;	*cc++ = 0xd8;		// add ebx, eax

			*cc++ = 0x8b; *cc++ = 0x1c; *cc++ = 0x19;	// mov	ebx, BYTE PTR [ecx+ebx]

			*cc++ = 0x88;	*cc++ = 0x9f; 
			*(uint *)cc = x; cc += 4;		// mov [edi+imm], bl

			u += du;
		}
	} else {
		// Pentium optimized code.

		*cc++ = 0xb8; *(uint *)cc = 0; cc += 4;		// mov eax, 0

		for (x=0; x<w; x++ )			{
			if ( last_u != f2i(u) )	{
				*cc++ = 0x8a;	*cc++ = 0xa6; // mov ah, [esi+imm]
				*(uint *)cc = f2i(u); cc += 4;
				last_u = f2i(u);
			}
			
  			*cc++ = 0x8a;	*cc++ = 0x87; 
			*(uint *)cc = x; cc += 4;		// mov al, [edi+imm]

			*cc++ = 0x8a; *cc++ = 0x1c; *cc++ = 0x01;	// mov	bl, BYTE PTR [ecx+eax]

			*cc++ = 0x88;	*cc++ = 0x9f; 
			*(uint *)cc = x; cc += 4;		// mov [edi+imm], bl

			u += du;
		}
	}

	*cc++ = 0xc3;	// RET

	if ( cc >= &compiled_code[MAX_CODE_SIZE] )
		Int3();		// GET JOHN NOW!

#ifdef FIND_MAX_SIZE
	int size = cc - compiled_code;
	if ( size > Max_size )	{
		Max_size = size;
		mprintf(( "Max size = %d\n", size ));
	}
#endif

	return compiled_code;
}

/*
				for (x=0; x<w; x++ )			{
					if ( fx_w > *zbuf )	{
						uint c = sbits[ tmp_u >> 16 ]<<8;
						*dbits = *((ubyte *)(lookup + (*dbits | c)));
					}
					dbits++;
					zbuf++;
					tmp_u += du;
				}
*/

//----------------------------------------------------
// scaler_create_compiled_code8_alpha_zbuffered
//
// Creates code that looks like:
// mov eax, 0
//     mov ah, [esi+????]   ; get the foreground pixel
//     ; the following lines might be repeated
//     cmp	fx_w, [edx+?????]
//     jle  @f
//     mov al, [edi+????]   ; get the background pixel
//     mov bl, [ecx+eax]	; blend it
//     mov [edi+????], bl   ; write it
//  @@:
//     ...




//void test_code1()
//{
//	_asm cmp 0xFFFFFFFF, [edx+0xabcdef12]
//	_asm cmp ebp, [edx+0xabcdef12]
//	_asm jle	0xabcdef12
//}
//; 302  : 	_asm cmp ebp, [edx+0xabcdef12]
//  00244	3b aa 12 ef cd ab		cmp	ebp, DWORD PTR [edx-1412567278]
//; 303  : 	_asm jle	0xabcdef12
//  0024a	0f 8e 12 ef cd ab		jle	-1412567278		; abcdef12H

ubyte *scaler_create_compiled_code8_alpha_zbuffered( int w, fix u, fix du )
{
	int last_u, x;
	ubyte * cc;
	uint *last_jmp_pos=NULL;

	cc = compiled_code;

	//     xor eax, eax			; avoid ppro partial register stall
//     mov ah, [esi+????]   ; get the foreground pixel
//     ; the following lines might be repeated
//     xor ebx, ebx			; avoid ppro partial register stall
//     mov bl, [edi+????]   ; get the background pixel
//     mov ebx, [ecx+ebx]	; blend it
//     mov [edi+????], bl   ; write it

	//if ( abs(du) < F1_0 / 4 ) *cc++ = 0xCC;

	//*cc++ = 0xCC;	// Int3
	//*cc++ = 0xc3;	// RET
	last_u = -1;

	if ( Gr_cpu	> 5 )	{
		// Pentium Pro optimized code.

		for (x=0; x<w; x++ )			{
			if ( last_u != f2i(u) )	{
				*cc++ = 0x33;	*cc++ = 0xc0; // xor eax, eax
				*cc++ = 0x8a;	*cc++ = 0xa6; // mov ah, [esi+imm]
				*(uint *)cc = f2i(u); cc += 4;
				last_u = f2i(u);
			}

			*cc++ = 0x3b;  *cc++ = 0xaa;	
			*(uint *)cc = x*4; cc += 4;		// cmp ebp, [edx+imm]

//			*cc++ = 0x3b;  *cc++ = 0x2a;						// cmp ebp, [edx]
//			*cc++ = 0x83;  *cc++ = 0xc2;  *cc++ = 0x4;	// add edx, 4

			*cc++ = 0x0f;  *cc++ = 0x8e;		// jle (8e) imm
			last_jmp_pos = (uint *)cc;
			*(uint *)cc = 0; cc += 4;
		
  			*cc++ = 0x33;	*cc++ = 0xdb;		// xor ebx, ebx
			
  			*cc++ = 0x8a;	*cc++ = 0x9f; 
			*(uint *)cc = x; cc += 4;		// mov bl, [edi+imm]

  			*cc++ = 0x03;	*cc++ = 0xd8;		// add ebx, eax

			*cc++ = 0x8b; *cc++ = 0x1c; *cc++ = 0x19;	// mov	ebx, BYTE PTR [ecx+ebx]

			*cc++ = 0x88;	*cc++ = 0x9f; 
			*(uint *)cc = x; cc += 4;		// mov [edi+imm], bl

			if ( last_jmp_pos )	{
				*last_jmp_pos = (uint)cc - (uint)last_jmp_pos - 4;
				last_jmp_pos  = NULL;
			}

			u += du;
		}


	} else {
		// Pentium optimized code.

		*cc++ = 0xb8; *(uint *)cc = 0; cc += 4;		// mov eax, 0

		for (x=0; x<w; x++ )			{
			if ( last_u != f2i(u) )	{
				*cc++ = 0x8a;	*cc++ = 0xa6; // mov ah, [esi+imm]
				*(uint *)cc = f2i(u); cc += 4;
				last_u = f2i(u);
			}

			*cc++ = 0x3b;  *cc++ = 0xaa;	
			*(uint *)cc = x*4; cc += 4;		// cmp ebp, [edx+imm]

			*cc++ = 0x0f;  *cc++ = 0x8e;		// jle imm
			last_jmp_pos = (uint *)cc;
			*(uint *)cc = 0; cc += 4;		
			
  			*cc++ = 0x8a;	*cc++ = 0x87; 
			*(uint *)cc = x; cc += 4;		// mov al, [edi+imm]

			*cc++ = 0x8a; *cc++ = 0x1c; *cc++ = 0x01;	// mov	bl, BYTE PTR [ecx+eax]

			*cc++ = 0x88;	*cc++ = 0x9f; 
			*(uint *)cc = x; cc += 4;		// mov [edi+imm], bl

			if ( last_jmp_pos )	{
				*last_jmp_pos = (uint)cc - (uint)last_jmp_pos - 4;
				last_jmp_pos = NULL;
			}

			u += du;
		}
	}
	*cc++ = 0xc3;	// RET

	if ( cc >= &compiled_code[MAX_CODE_SIZE] )
		Int3();		// GET JOHN NOW!

#ifdef FIND_MAX_SIZE
	int size = cc - compiled_code;
	if ( size > Max_size )	{
		Max_size = size;
		mprintf(( "Max sizeZ = %d\n", size ));
	}
#endif

	return compiled_code;
}



int Gr_scaler_zbuffering = 0;
uint Gr_global_z;

MONITOR( ScalerNumCalls );	


//----------------------------------------------------
// Scales current bitmap, between va and vb
void gr8_scaler(vertex *va, vertex *vb )
{
#if 1
	if(Pofview_running){
		return;
	}

	float x0, y0, x1, y1;
	float u0, v0, u1, v1;
	float clipped_x0, clipped_y0, clipped_x1, clipped_y1;
	float clipped_u0, clipped_v0, clipped_u1, clipped_v1;
	float xmin, xmax, ymin, ymax;
	int dx0, dy0, dx1, dy1;

	MONITOR_INC( ScalerNumCalls, 1 );	

	//============= CLIP IT =====================

	x0 = va->sx; y0 = va->sy;
	x1 = vb->sx; y1 = vb->sy;

	xmin = i2fl(gr_screen.clip_left); ymin = i2fl(gr_screen.clip_top);
	xmax = i2fl(gr_screen.clip_right); ymax = i2fl(gr_screen.clip_bottom);

	u0 = va->u; v0 = va->v;
	u1 = vb->u; v1 = vb->v;

	// Check for obviously offscreen bitmaps...
	if ( (y1<=y0) || (x1<=x0) ) return;
	if ( (x1<xmin ) || (x0>xmax) ) return;
	if ( (y1<ymin ) || (y0>ymax) ) return;

	clipped_u0 = u0; clipped_v0 = v0;
	clipped_u1 = u1; clipped_v1 = v1;

	clipped_x0 = x0; clipped_y0 = y0;
	clipped_x1 = x1; clipped_y1 = y1;

	// Clip the left, moving u0 right as necessary
	if ( x0 < xmin ) 	{
		clipped_u0 = FIND_SCALED_NUM(xmin,x0,x1,u0,u1);
		clipped_x0 = xmin;
	}

	// Clip the right, moving u1 left as necessary
	if ( x1 > xmax )	{
		clipped_u1 = FIND_SCALED_NUM(xmax,x0,x1,u0,u1);
		clipped_x1 = xmax;
	}

	// Clip the top, moving v0 down as necessary
	if ( y0 < ymin ) 	{
		clipped_v0 = FIND_SCALED_NUM(ymin,y0,y1,v0,v1);
		clipped_y0 = ymin;
	}

	// Clip the bottom, moving v1 up as necessary
	if ( y1 > ymax ) 	{
		clipped_v1 = FIND_SCALED_NUM(ymax,y0,y1,v0,v1);
		clipped_y1 = ymax;
	}
	
	dx0 = fl2i(clipped_x0); dx1 = fl2i(clipped_x1);
	dy0 = fl2i(clipped_y0); dy1 = fl2i(clipped_y1);

	if (dx1<=dx0) return;
	if (dy1<=dy0) return;

	//============= DRAW IT =====================
	int u, v, du, dv;
	int y, w;
	ubyte * sbits, * dbits;
	bitmap * bp;
	ubyte * spixels;
	float tmpu, tmpv;

	tmpu = (clipped_u1-clipped_u0) / (dx1-dx0);
	if ( fl_abs(tmpu) < MIN_SCALE_FACTOR ) {
		return;		// scaled up way too far!
	}
	tmpv = (clipped_v1-clipped_v0) / (dy1-dy0);
	if ( fl_abs(tmpv) < MIN_SCALE_FACTOR ) {
		return;		// scaled up way too far!
	}

	int is_stippled = 0;

	/*
	if ( !Detail.alpha_effects )	{
		is_stippled = 1;
		Gr_scaler_zbuffering = 0;
	}
	*/
	
	if ( is_stippled )	{
		bp = bm_lock( gr_screen.current_bitmap, 8, 0 );
	} else {
		bp = bm_lock( gr_screen.current_bitmap, 8, 0 );
	}


	du = fl2f(tmpu*(bp->w-1));
	dv = fl2f(tmpv*(bp->h-1));

	v = fl2f(clipped_v0*(bp->h-1));
	u = fl2f(clipped_u0*(bp->w-1)); 
	w = dx1 - dx0 + 1;
	if ( w < 2 ) {
		bm_unlock(gr_screen.current_bitmap);
		return;
	}

	uint fx_w = 0;
	if ( Gr_scaler_zbuffering && gr_zbuffering )	{
		fx_w = (uint)fl2i(va->sw * GR_Z_RANGE)+gr_zoffset;
		Gr_global_z = fx_w;
	}

#ifdef USE_COMPILED_CODE
	ubyte *cc=NULL;

	if ( Gr_scaler_zbuffering && gr_zbuffering )	{
		if ( gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER )	{
			cc = scaler_create_compiled_code8_alpha_zbuffered( w, u, du );	
		}
	} else {
		if ( gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER )	{
			if ( is_stippled )	{
				cc = scaler_create_compiled_code8_stippled( w, u, du );
			} else {
				cc = scaler_create_compiled_code8_alpha( w, u, du );	
			}
		} else	{
			cc = scaler_create_compiled_code8( w, u, du );
		}
	}
	
#endif

	spixels = (ubyte *)bp->data;

	gr_lock();
	Tmap.pScreenBits = (uint)gr_screen.offscreen_buffer_base;

	uint *zbuf;

	for (y=dy0; y<=dy1; v += dv, y++ )			{
		if ( is_stippled && (y&1) )	{
			sbits = &spixels[bp->rowsize*(v>>16)+f2i(du)];
			dbits = GR_SCREEN_PTR(ubyte,dx0+1,y);
		} else {
			sbits = &spixels[bp->rowsize*(v>>16)];
			dbits = GR_SCREEN_PTR(ubyte,dx0,y);
		}
		uint lookup = 0;

		if ( gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER )	{
			lookup = (uint)palette_get_blend_table(gr_screen.current_alpha);
		}

		if ( Gr_scaler_zbuffering && gr_zbuffering )	{
			zbuf = (uint *)&gr_zbuffer[(uint)dbits-(uint)Tmap.pScreenBits];
		}
	
#ifdef USE_COMPILED_CODE
		// Call the compiled code to draw one scanline
		if ( Gr_scaler_zbuffering &&  gr_zbuffering && (gr_screen.current_alphablend_mode != GR_ALPHABLEND_FILTER))	{			
			Int3();

			/*
			int x, tmp_u;
			tmp_u = u;

			for (x=0; x<w; x++ )			{
				if ( fx_w > *zbuf )	{
					ubyte c = sbits[ tmp_u >> 16 ];
					if ( c != TRANSPARENCY_COLOR_8 ) *dbits = c;
				}
				zbuf++;
				dbits++;
				tmp_u += du;
			}
			*/
		} else {
/*			{
				int x, tmp_u;
				tmp_u = u;

	
				for (x=0; x<w; x++ )			{
					if ( fx_w > *zbuf )	{
						uint c = sbits[ tmp_u >> 16 ]<<8;
						*dbits = *((ubyte *)(lookup + (*dbits | c)));
					}
					dbits++;
					zbuf++;
					tmp_u += du;
				}
			} 
*/
			_asm push esi
			_asm push edi
			_asm push edx
			_asm push ecx
			_asm push ebx
			_asm push eax
			_asm mov ecx, lookup
			_asm mov esi, sbits
			_asm mov edi, dbits
			_asm mov eax, cc
			_asm mov edx, zbuf
			_asm push ebp
			_asm mov ebp, Gr_global_z
			_asm call eax
			_asm pop ebp
			_asm pop eax
			_asm pop ebx
			_asm pop ecx
			_asm pop edx
			_asm pop edi
			_asm pop esi
		}
#else	
		if ( gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER )	{
			if ( Gr_scaler_zbuffering && gr_zbuffering )	{
				int x, tmp_u;
				tmp_u = u;

				for (x=0; x<w; x++ )			{
					if ( fx_w > *zbuf )	{
						uint c = sbits[ tmp_u >> 16 ]<<8;
						*dbits = *((ubyte *)(lookup + (*dbits | c)));
					}
					dbits++;
					zbuf++;
					tmp_u += du;
				}
			} else {
				int x, tmp_u;
				tmp_u = u;
				for (x=0; x<w; x++ )			{
					uint c = sbits[ tmp_u >> 16 ]<<8;
					*dbits++ = palette_blend[*dbits|c];
					tmp_u += du;
				}
			}
		} else {
			if ( Gr_scaler_zbuffering && gr_zbuffering )	{
				int x, tmp_u;
				tmp_u = u;
			
				for (x=0; x<w; x++ )			{
					if ( fx_w > *zbuf )	{
						ubyte c = sbits[ tmp_u >> 16 ];
						if ( c != TRANSPARENCY_COLOR_8 ) *dbits = c;
					}
					zbuf++;
					dbits++;
					tmp_u += du;
				}
			} else {
				int x, tmp_u;
				tmp_u = u;
				for (x=0; x<w; x++ )			{
					ubyte c = sbits[ tmp_u >> 16 ];
					if ( c != TRANSPARENCY_COLOR_8 ) *dbits = c;
					dbits++;
					tmp_u += du;
				}
			}
		}
#endif
	}

	gr_unlock();
	bm_unlock(gr_screen.current_bitmap);
#endif
}

int aiee = 0;
alphacolor_old old_alphac;
//----------------------------------------------------
// Scales current bitmap, between va and vb
void gr8_aascaler(vertex *va, vertex *vb )
{
	float x0, y0, x1, y1;
	float u0, v0, u1, v1;
	float clipped_x0, clipped_y0, clipped_x1, clipped_y1;
	float clipped_u0, clipped_v0, clipped_u1, clipped_v1;
	float xmin, xmax, ymin, ymax;
	int dx0, dy0, dx1, dy1;

	//if ( !Current_alphacolor )	return;

	MONITOR_INC( ScalerNumCalls, 1 );	

	Assert(Fred_running);
	if(!aiee){
		old_alphac.used = 1;
		old_alphac.r = 93;
		old_alphac.g = 93;
		old_alphac.b = 128;
		old_alphac.alpha = 255;
		//ac->type = type;
		//ac->clr=clr;
		//93, 93, 128, 255
		calc_alphacolor_old(&old_alphac);
		aiee = 1;
	}

	//============= CLIP IT =====================

	x0 = va->sx; y0 = va->sy;
	x1 = vb->sx; y1 = vb->sy;

	xmin = i2fl(gr_screen.clip_left); ymin = i2fl(gr_screen.clip_top);
	xmax = i2fl(gr_screen.clip_right); ymax = i2fl(gr_screen.clip_bottom);

	u0 = va->u; v0 = va->v;
	u1 = vb->u; v1 = vb->v;

	// Check for obviously offscreen bitmaps...
	if ( (y1<=y0) || (x1<=x0) ) return;
	if ( (x1<xmin ) || (x0>xmax) ) return;
	if ( (y1<ymin ) || (y0>ymax) ) return;

	clipped_u0 = u0; clipped_v0 = v0;
	clipped_u1 = u1; clipped_v1 = v1;

	clipped_x0 = x0; clipped_y0 = y0;
	clipped_x1 = x1; clipped_y1 = y1;

	// Clip the left, moving u0 right as necessary
	if ( x0 < xmin ) 	{
		clipped_u0 = FIND_SCALED_NUM(xmin,x0,x1,u0,u1);
		clipped_x0 = xmin;
	}

	// Clip the right, moving u1 left as necessary
	if ( x1 > xmax )	{
		clipped_u1 = FIND_SCALED_NUM(xmax,x0,x1,u0,u1);
		clipped_x1 = xmax;
	}

	// Clip the top, moving v0 down as necessary
	if ( y0 < ymin ) 	{
		clipped_v0 = FIND_SCALED_NUM(ymin,y0,y1,v0,v1);
		clipped_y0 = ymin;
	}

	// Clip the bottom, moving v1 up as necessary
	if ( y1 > ymax ) 	{
		clipped_v1 = FIND_SCALED_NUM(ymax,y0,y1,v0,v1);
		clipped_y1 = ymax;
	}
	
	dx0 = fl2i(clipped_x0); dx1 = fl2i(clipped_x1);
	dy0 = fl2i(clipped_y0); dy1 = fl2i(clipped_y1);

	if (dx1<=dx0) return;
	if (dy1<=dy0) return;

	//============= DRAW IT =====================
	int u, v, du, dv;
	int y, w;
	ubyte * sbits, * dbits;
	bitmap * bp;
	ubyte * spixels;
	float tmpu, tmpv;

	tmpu = (clipped_u1-clipped_u0) / (dx1-dx0);
	if ( fl_abs(tmpu) < MIN_SCALE_FACTOR ) {
		return;		// scaled up way too far!
	}
	tmpv = (clipped_v1-clipped_v0) / (dy1-dy0);
	if ( fl_abs(tmpv) < MIN_SCALE_FACTOR ) {
		return;		// scaled up way too far!
	}

	bp = bm_lock( gr_screen.current_bitmap, 8, BMP_AABITMAP );

	du = fl2f(tmpu*(bp->w-1));
	dv = fl2f(tmpv*(bp->h-1));

	v = fl2f(clipped_v0*(bp->h-1));
	u = fl2f(clipped_u0*(bp->w-1)); 
	w = dx1 - dx0 + 1;

#ifdef USE_COMPILED_CODE
	ubyte *cc;

	if ( Gr_scaler_zbuffering && gr_zbuffering )	{
		//cc = scaler_create_compiled_code8_alpha_zbuffered( w, u, du );
	} else {
		cc = scaler_create_compiled_code8_alpha( w, u, du );
	}

#endif

	spixels = (ubyte *)bp->data;

	gr_lock();

	uint fx_w = 0;
	if ( Gr_scaler_zbuffering  && gr_zbuffering )	{
		fx_w = (uint)fl2i(va->sw * GR_Z_RANGE)+gr_zoffset;
	}	

	for (y=dy0; y<=dy1; y++ )			{
		sbits = &spixels[bp->rowsize*(v>>16)];
		dbits = GR_SCREEN_PTR(ubyte,dx0,y);
		// uint lookup = (uint)&Current_alphacolor->table.lookup[0][0];
		uint lookup = (uint)&old_alphac.table.lookup[0][0];
		
#ifdef USE_COMPILED_CODE
		// Call the compiled code to draw one scanline
		if ( Gr_scaler_zbuffering  && gr_zbuffering )	{
			int x, tmp_u;
			tmp_u = u;

			uint *zbuf = (uint *)&gr_zbuffer[(uint)dbits-(uint)Tmap.pScreenBits];
	
			for (x=0; x<w; x++ )			{
				if ( fx_w > *zbuf )	{
					// uint c = sbits[ tmp_u >> 16 ];
					// *dbits = Current_alphacolor->table.lookup[c][*dbits];
					*dbits = (ubyte)0x00;
				}
				zbuf++;
				dbits++;
				tmp_u += du;
			}
		} else {
			_asm push esi
			_asm push edi
			_asm push ecx
			_asm push ebx
			_asm push eax
			_asm mov ecx, lookup
			_asm mov esi, sbits
			_asm mov edi, dbits
			_asm mov eax, cc
			_asm call eax
			_asm pop eax
			_asm pop ebx
			_asm pop ecx
			_asm pop edi
			_asm pop esi
		}
#else	
		if ( Gr_scaler_zbuffering && gr_zbuffering )	{
			int x, tmp_u;
			tmp_u = u;

			uint *zbuf = (uint *)&gr_zbuffer[(uint)dbits-(uint)Tmap.pScreenBits];
	
			for (x=0; x<w; x++ )			{
				if ( fx_w > *zbuf )	{
					uint c = sbits[ tmp_u >> 16 ];
					*dbits = Current_alphacolor->table.lookup[c][*dbits];
				}
				zbuf++;
				dbits++;
				tmp_u += du;
			}
		} else {
			int x, tmp_u;
			tmp_u = u;
			for (x=0; x<w; x++ )			{
				uint c = sbits[ tmp_u >> 16 ];
				*dbits = Current_alphacolor->table.lookup[c][*dbits];
				dbits++;
				tmp_u += du;
			}
		}
#endif
		v += dv;
	}

	gr_unlock();

	bm_unlock(gr_screen.current_bitmap);
}
