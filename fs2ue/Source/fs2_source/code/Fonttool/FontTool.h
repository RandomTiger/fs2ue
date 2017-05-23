/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _FONTTOOL_H
#define _FONTTOOL_H

#include "2d.h"
#include "font.h"

void fonttool_edit_kerning(char *fname1);
void fonttool_kerning_copy( char *fname1, char *fname2 );
void fonttool_create_font(char *pcx_filename, char *font_filename);

void fonttool_read( char *fname2, font *fnt2 );
void fonttool_copy_kern( font *fnt1, font *fnt2 );
void fonttool_dump( char *fname1, font *fnt1 );
void fonttool_remove_kerning( font *fnt );
void fonttool_resync_kerning(font *fnt);


#endif
