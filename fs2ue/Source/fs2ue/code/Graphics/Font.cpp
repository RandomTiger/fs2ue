/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#ifndef UNITY_BUILD
#include "font.h"

//#include <windows.h>
//#include <windowsx.h>
#include <stdio.h>
#include <stdarg.h>
#include "GrInternal.h"
#include "2d.h"
#include "cfile.h"
#include "PalMan.h"
#include "key.h"
#include "BmpMan.h"
#include "Localize.h"
#endif

int Num_fonts = 0;
font Fonts[MAX_FONTS];
font *Current_font = NULL;


// crops a string if required to force it to not exceed max_width pixels when printed.
// Does this by dropping characters at the end of the string and adding '...' to the end.
//    str = string to crop.  Modifies this string directly
//    max_str = max characters allowed in str
//    max_width = number of pixels to limit string to (less than or equal to).
//    Returns: returns same pointer passed in for str.
char *gr_force_fit_string(char *str, int max_str, int max_width)
{
	int w;

	gr_get_string_size(&w, NULL, str);
	if (w > max_width) {
		if ((int) strlen(str) > max_str - 3) {
			Assert(max_str >= 3);
			str[max_str - 3] = 0;
		}

		strcpy(str + strlen(str) - 1, "...");
		gr_get_string_size(&w, NULL, str);
		while (w > max_width) {
			Assert(strlen(str) >= 4);  // if this is hit, a bad max_width was passed in and the calling function needs fixing.
			strcpy(str + strlen(str) - 4, "...");
			gr_get_string_size(&w, NULL, str);
		}
	}

	return str;
}

//takes the character BEFORE being offset into current font
// Returns the letter code
int get_char_width(ubyte c1,ubyte c2,int *width,int *spacing)
{
	int i, letter;

	Assert ( Current_font != NULL );
	letter = c1-Current_font->first_ascii;

	if (letter<0 || letter>=Current_font->num_chars) {				//not in font, draw as space
		*width=0;
		*spacing = Current_font->w;
		return -1;
	}

	*width = Current_font->char_data[letter].byte_width;
	*spacing = Current_font->char_data[letter].spacing;

	i = Current_font->char_data[letter].kerning_entry;
	if ( i > -1)  {
		if (!(c2==0 || c2=='\n')) {
			int letter2;

			letter2 = c2-Current_font->first_ascii;

			if ((letter2>=0) && (letter2<Current_font->num_chars) ) {				//not in font, draw as space
				font_kernpair	*k = &Current_font->kern_data[i];
				while( (k->c1 == letter) && (k->c2<letter2) && (i<Current_font->num_kern_pairs) )	{
					i++;
					k++;
				}
				if ( k->c2 == letter2 )	{
					*spacing += k->offset;
				}
			}
		}
	}
	return letter;
}

int get_centered_x(const char * const lpString)
{
	int w,w2,s2;
	const char *s = lpString;

	for (w=0;*s!=0 && *s!='\n';s++) {
		get_char_width(s[0],s[1],&w2,&s2);
		w += s2;
	}

	return ((gr_screen.clip_width - w) / 2);
}

// draws a character centered on x
void gr_char_centered(int x, int y, char chr)
{
	char str[2];
	int w, sc;

	sc = Lcl_special_chars;
	if (chr == '1')
		chr = (char)(sc + 1);

	str[0] = chr;
	str[1] = 0;
	gr_get_string_size(&w, NULL, str);
	gr_string(x - w / 2, y, str);
}

void gr_print_timestamp(int x, int y, int timestamp)
{
	char h[2], m[3], s[3];
	int w, c;

	// format the time information into strings
	sprintf(h, "%0.1d", (timestamp / 3600000) % 10);
	sprintf(m, "%0.2d", (timestamp / 60000) % 60);
	sprintf(s, "%0.2d", (timestamp / 1000) % 60);

	gr_get_string_size(&w, NULL, "0");
	gr_get_string_size(&c, NULL, ":");

	gr_string(x + w, y, ":");
	gr_string(x + w * 3 + c, y, ":");

	x += w / 2;
	gr_char_centered(x, y, h[0]);
	x += w + c;
	gr_char_centered(x, y, m[0]);
	x += w;
	gr_char_centered(x, y, m[1]);
	x += w + c;
	gr_char_centered(x, y, s[0]);
	x += w;
	gr_char_centered(x, y, s[1]);
}

int gr_get_font_height()
{
	if (Current_font)	{
		return Current_font->h;
	} else {
		return 16;
	}
}

void gr_get_string_size(int *w1, int *h1, char *text, int len)
{
	int longest_width;
	int width,spacing;
	int w, h;

	if (!Current_font)	{
		if ( w1)
			*w1 = 16;

		if ( h1 )
			*h1 = 16;

		return;
	}	

	w = 0;
	h = 0;
	longest_width = 0;

	if (text != NULL ) {
		h += Current_font->h;
		while (*text && (len>0) ) {

			// Process one or more 
			while ((*text == '\n') && (len>0) ) {
				text++;
				len--;
				if ( *text )	{
					h += Current_font->h;
				}
				w = 0;
			}

			if (*text == 0)	{
				break;
			}

			get_char_width(text[0], text[1], &width, &spacing);
			w += spacing;
			if (w > longest_width)
				longest_width = w;

			text++;
			len--;
		}
	}

	if ( h1 )
		*h1 = h;

	if ( w1 )
		*w1 = longest_width;
}

MONITOR( FontChars );	

char grx_printf_text[2048];	

void _cdecl gr_printf( int x, int y, char * format, ... )
{
	va_list args;

	if ( !Current_font ) return;
	
	va_start(args, format);
	vsprintf(grx_printf_text,format,args);
	va_end(args);

	gr_string(x,y,grx_printf_text);
}

void gr_font_close()
{

}

// Returns -1 if couldn't init font, otherwise returns the
// font id number.
int gr_create_font(char * typeface)
{
	const int fontBpp = 8;
	CFILE *fp;
	font *fnt;
	int n, fontnum;

	fnt = Fonts;
	n = -1;
	for (fontnum=0; fontnum<Num_fonts; fontnum++ )	{
		if (fnt->id != 0 )	{
			if ( !_strnicmp( fnt->filename, typeface, MAX_FILENAME_LEN ) )	{
				return fontnum;
			}
		} else {
			if ( n < 0 )	{
				n = fontnum;
			}				
		}
		fnt++;
	}

	if ( fontnum==MAX_FONTS )	{
		Error( LOCATION, "Too many fonts!\nSee John, or change MAX_FONTS in Graphics\\Font.h\n" );
	}

	if ( fontnum == Num_fonts )	{
		Num_fonts++;
	}
	
	bool localize = true;

	fp = cfopen( typeface, "rb", CFILE_NORMAL, CF_TYPE_ANY, localize );
	if ( fp == NULL ) return -1;

	strncpy( fnt->filename, typeface, MAX_FILENAME_LEN );
	cfread( &fnt->id, 4, 1, fp );
	cfread( &fnt->version, sizeof(int), 1, fp );
	cfread( &fnt->num_chars, sizeof(int), 1, fp );
	cfread( &fnt->first_ascii, sizeof(int), 1, fp );
	cfread( &fnt->w, sizeof(int), 1, fp );
	cfread( &fnt->h, sizeof(int), 1, fp );
	cfread( &fnt->num_kern_pairs, sizeof(int), 1, fp );
	cfread( &fnt->kern_data_size, sizeof(int), 1, fp );
	cfread( &fnt->char_data_size, sizeof(int), 1, fp );
	cfread( &fnt->pixel_data_size, sizeof(int), 1, fp );

	if ( fnt->kern_data_size )	{
		fnt->kern_data = (font_kernpair *)malloc( fnt->kern_data_size );
		Assert(fnt->kern_data!=NULL);
		cfread( fnt->kern_data, fnt->kern_data_size, 1, fp );
	} else {
		fnt->kern_data = NULL;
	}
	if ( fnt->char_data_size )	{
		fnt->char_data = (font_char *)malloc( fnt->char_data_size );
		Assert( fnt->char_data != NULL );
		cfread( fnt->char_data, fnt->char_data_size, 1, fp );
	} else {
		fnt->char_data = NULL;
	}
	if ( fnt->pixel_data_size )	{
		fnt->pixel_data = (ubyte *)malloc( fnt->pixel_data_size );
		Assert(fnt->pixel_data!=NULL);
		cfread( fnt->pixel_data, fnt->pixel_data_size, 1, fp );
	} else {
		fnt->pixel_data = NULL;
	}
	cfclose(fp);

	// Create a bitmap for hardware cards.
	// JAS:  Try to squeeze this into the smallest square power of two texture.
	// This should probably be done at font generation time, not here.
	int w, h;
	if ( fnt->pixel_data_size < 64*64 )	{
		w = h = 64;
	} else if ( fnt->pixel_data_size < 128*128 )	{
		w = h = 128;
	} else {
		w = h = 256;
	}

	fnt->bm_w = w;
	fnt->bm_h = h;
	fnt->bm_data = (ubyte *)malloc(fnt->bm_w*fnt->bm_h * fontBpp / 8);
	fnt->bm_u = (int *)malloc(sizeof(int)*fnt->num_chars);
	fnt->bm_v = (int *)malloc(sizeof(int)*fnt->num_chars);

	int i,x,y;
	x = y = 0;
	for (i=0; i<fnt->num_chars; i++ )	{
		ubyte * fp;
		int x1, y1;
		fp = &fnt->pixel_data[fnt->char_data[i].offset];
		if ( x + fnt->char_data[i].byte_width >= fnt->bm_w )	{
			x = 0;
			y += fnt->h;
			if ( y+fnt->h > fnt->bm_h ) {
				Error( LOCATION, "Font too big!\n" );
			}
		}
		fnt->bm_u[i] = x;
		fnt->bm_v[i] = y;

		for( y1=0; y1<fnt->h; y1++ )	{
			for (x1=0; x1<fnt->char_data[i].byte_width; x1++ )	{
				uint c = *fp++;
				if ( c > 14 ) c = 14;

				const int index = (x+x1)+(y+y1)*fnt->bm_w * fontBpp / 8;
				const unsigned char value = unsigned char(c);

				fnt->bm_data[index + 0] = value;	
				if(fontBpp == 32)
				{
					fnt->bm_data[index + 1] = value;	
					fnt->bm_data[index + 2] = value;	
					fnt->bm_data[index + 3] = 255;
				}
			}
		}
		x += fnt->char_data[i].byte_width;
	}

	fnt->bitmap_id = bm_create( fontBpp, fnt->bm_w, fnt->bm_h, fnt->bm_data, fontBpp == 8 ? BMP_AABITMAP : 0);

	return fontnum;
}

void grx_set_font(int fontnum)
{
	if ( fontnum < 0 ) {
		Current_font = NULL;
		return;
	}

	if ( fontnum >= 0 ) {
		Current_font = &Fonts[fontnum];
	}
}

void gr_font_init()
{
	Num_fonts = 0;

	gr_init_font( NOX("font01.vf") );
	gr_init_font( NOX("font02.vf") );
	gr_init_font( NOX("font03.vf") );
}

// Returns -1 if couldn't init font, otherwise returns the
// font id number.
int gr_init_font(char * typeface)
{
	int Loaded_fontnum;

	Loaded_fontnum = gr_create_font(typeface);

	Assert( Loaded_fontnum > -1 );

	gr_set_font( Loaded_fontnum );

	return Loaded_fontnum;
}

void gr_font_deinit()
{
	for (int fontnum=0; fontnum < Num_fonts; fontnum++ )
	{
		free(Fonts[fontnum].char_data);
		free(Fonts[fontnum].kern_data);
		free(Fonts[fontnum].pixel_data);
		free(Fonts[fontnum].bm_data);
		free(Fonts[fontnum].bm_u);
		free(Fonts[fontnum].bm_v);
	}

}