/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "pstypes.h"

#ifndef __CONVERT_H__
#define __CONVERT_H__

// !!IMPORTANT!!
// ANIM_HEADER_SIZE is the size (in bytes) of the header.  If the header format changes, you
// must update this number.
#define ANIM_HEADER_SIZE	790

#define ANIM_BUFFER_MAX 400000
#define ANIM_VERSION		2				// update this when save format changes

#define ANIM_DEFAULT_FPS	15

// specify the different types of compression that can be used (Compression_type is assigned one of these values)
#define	CUSTOM_DELTA_RLE	0
#define	STD_DELTA_RLE		1

int convert_avi_to_anim(char* filename);
int convert_frames_to_anim(char *filename);

typedef struct rgb_triple {
	ubyte	r, g, b;
} rgb_triple;

extern int			key_frame_rate;
extern int			force_key_frame;
extern int			Default_fps;
extern rgb_triple	Xparent_color;
extern int			Use_custom_xparent_color;
extern int			Compression_type;


#endif /* __CONVERT_H__ */