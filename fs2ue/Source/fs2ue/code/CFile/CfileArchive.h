/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _CFILEARCHIVE_H
#define _CFILEARCHIVE_H

#ifndef UNITY_BUILD
#ifndef _CFILE_INTERNAL 
#error This file should only be included internally in CFILE!!
#endif
#endif

typedef void *HANDLE;

// The following Cfile_block data is private to cfile.cpp
// DO NOT MOVE the Cfile_block* information to cfile.h / do not extern this data
//
#define CFILE_BLOCK_UNUSED		0
#define CFILE_BLOCK_USED		1

typedef struct Cfile_block {
	int		type;				// CFILE_BLOCK_UNUSED, CFILE_BLOCK_USED
	int		dir_type;		// directory location
	FILE		*fp;				// File pointer if opening an individual file
	void		*data;			// Pointer for memory-mapped file access.  NULL if not mem-mapped.
	HANDLE	hInFile;			// Handle from CreateFile()
	HANDLE	hMapFile;		// Handle from CreateFileMapping()
	int		lib_offset;
	int		raw_position;
	int		size;				// for packed files
	
} Cfile_block;

struct CFILE;

#define MAX_CFILE_BLOCKS	64
extern Cfile_block Cfile_block_list[MAX_CFILE_BLOCKS];
extern CFILE Cfile_list[MAX_CFILE_BLOCKS];

// Called once to setup the low-level reading code.
void cf_init_lowlevel_read_code( CFILE * cfile, int offset, int size );

#endif