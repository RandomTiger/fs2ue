/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _PSTYPES_H
#define _PSTYPES_H

class GlobalConst
{
public:
	static const bool kCDCheckOn = false;
};


// Build defines.  Comment in/out for whatever build is necessary:
// #define OEM_BUILD						// enable for OEM builds
// #define MULTIPLAYER_BETA_BUILD				// enable for multiplayer beta build
// #define E3_BUILD							// enable for 3dfx E3 build						
// #define PRESS_TOUR_BUILD			// enable for press tour build
// #define FS2_DEMO					// enable demo build for FS2
// #define PD_BUILD						// fred documentation/evaluation build
//	#define FRENCH_BUILD				// build for French (obsolete)
// #define GERMAN_BUILD				// build for German (this is now used)
#define RELEASE_REAL					// this means that it is an actual release candidate, not just an optimized/release build

// uncomment this #define for DVD version (makes popups say DVD instead of CD 2 or whatever): JCF 5/10/2000
// #define DVD_MESSAGE_HACK


#if defined(MULTIPLAYER_BETA_BUILD) || defined(E3_BUILD) || defined(RELEASE_REAL)
//	#define GAME_CD_CHECK
#endif

inline void Unused(int i)
{
	i = 0;
}

#ifndef UNITY_BUILD
#include <stdlib.h>
#include <string.h>
#include <stdio.h>	// For NULL, etc
#include "Pragma.h"
#include <memory.h>
#include <malloc.h>
#include "Math/vector.h"
#include "Math/matrix.h"
#include "OsApi/OutWnd.h"
#include "GlobalIncs/SafeArray.h"
#endif

// value to represent an uninitialized state in any int or uint
#define UNINITIALIZED 0x7f8e6d9c

#if defined(DEMO) || defined(OEM_BUILD) // no change for FS2_DEMO
	#define MAX_PLAYERS	1
#else
	#define MAX_PLAYERS	12
#endif

#define MAX_TEAMS		3

#define USE_INLINE_ASM 1		// Define this to use inline assembly
#define STRUCT_CMP(a, b) memcmp((void *) &a, (void *) &b, sizeof(a))

typedef __int64 longlong;
typedef long fix;
typedef unsigned char ubyte;
typedef unsigned short ushort;
typedef unsigned int uint;
/*
typedef unsigned int U32;
typedef signed int S32;
typedef unsigned short U16;
typedef signed short S16;
typedef unsigned char U8;
typedef signed char S8;
typedef float FP32;
typedef double FP64;
*/
#define HARDWARE_ONLY

//Stucture to store clipping codes in a word
typedef struct ccodes {
	ubyte or,and;	//or is low byte, and is high byte
} ccodes;

// A vector referenced as an array
typedef struct vectora {
	float	xyz[3];
} vectora;

typedef struct angles {
	float	p, b, h;
} angles;

typedef struct uv_pair {
	float u,v;
} uv_pair;

// Used to store rotated points for mines.
// Has flag to indicate if projected.
typedef struct vertex {
	float		x, y, z;				// world space position
	float		sx, sy, sw;			// screen space position (sw == 1/z)
	float		u, v;					// texture position
	ubyte		r, g, b, a;			// color.  Use b for darkening;
	ubyte		codes;				// what sides of view pyramid this point is on/off.  0 = Inside view pyramid.
	ubyte		flags;				// Projection flags.  Indicates whether it is projected or not or if projection overflowed.
	ubyte		pad[2];				// pad structure to be 4 byte aligned.
} vertex;

enum
{
	BMP_AABITMAP	=	(1<<0),				// antialiased bitmap
	BMP_TEX_XPARENT	=	(1<<1),				// transparent texture
	BMP_TEX_OTHER	=	(1<<3),				// so we can identify all "normal" textures

	// any texture type
	BMP_TEX_ANY		=	( BMP_TEX_XPARENT | BMP_TEX_OTHER ),
};

typedef struct bitmap {
	short	w, h;		// Width and height
	short	rowsize;	// What you need to add to go to next row
	ubyte	bpp;		// How many bits per pixel it is. (7,8,15,16,24,32)
	ubyte	flags;	// See the BMP_???? defines for values
	void   *data;		// Pointer to data, or maybe offset into VRAM.
	ubyte *palette;	// If bpp==8, this is pointer to palette.   If the BMP_NO_PALETTE_MAP flag
							// is not set, this palette just points to the screen palette. (gr_palette)

	uint GetDataAsUint() {return (uint) data;}
} bitmap;

//This are defined in MainWin.c
extern void _cdecl WinAssert(char * text,char *filename, int line);
extern void _cdecl Error( char * filename, int line, char * format, ... );
extern void _cdecl Warning( char * filename, int line, char * format, ... );

// To debug printf do this:
// mprintf(( "Error opening %s\n", filename ));
#ifndef NDEBUG
#define mprintf(args) outwnd_printf2 args
#define nprintf(args) outwnd_printf args
#else
#define mprintf(args) Unused(0)
#define nprintf(args) Unused(0)
#endif

#define LOCATION __FILE__,__LINE__

// To flag an error, you can do this:
// Error( __FILE__, __LINE__, "Error opening %s", filename );
// or, 
// Error( LOCATION, "Error opening %s", filename );

#if defined(NDEBUG)
#define Assert(x) Unused((x) == 0)
#else
void gr_activate(int);
#define Assert(x) do { if (!(x)){ gr_activate(0); WinAssert(#x,__FILE__,__LINE__); gr_activate(1); } } while (0)
#endif

//#define Int3() _asm { int 3 }

#ifdef INTERPLAYQA
	// Interplay QA version of Int3
	#define Int3() do { } while (0) 

	// define to call from Warning function above since it calls Int3, so without this, we
	// get put into infinite dialog boxes
	#define AsmInt3() _asm { int 3 }

#else
	#if defined(NDEBUG)
		// No debug version of Int3
		#define Int3() do { } while (0) 
	#else
		void debug_int3();

		// Debug version of Int3
		#define Int3() debug_int3()
	#endif	// NDEBUG && DEMO
#endif	// INTERPLAYQA

#ifndef UNITY_BUILD
template <class T> T min(const T& a, const T& b) {return (a < b) ? a : b; }

template <class T> T max(const T& a, const T& b) {return (a > b) ? a : b; }
#endif

const float PI	= 3.141592654f;
const float PI2 = PI * 2.0f;	// PI*2
inline float ANG_TO_RAD(const float x) { return x * PI / 180.0f;}


extern int	Fred_running;  // Is Fred running, or FreeSpace?
extern int Pofview_running;
extern int Nebedit_running;


//======================================================================================
//======          D E B U G    C O N S O L E   S T U F F        ========================
//======================================================================================

// Here is a a sample command to toggle something that would
// be called by doing "toggle it" from the debug console command window/

/*
DCF(toggle_it,"description")
{
	if (Dc_command)	{
		This_var = !This_var;
	}

	if (Dc_help)	{
		dc_printf( "Usage: sample\nToggles This_var on/off.\n" );
	}

	if (Dc_status)	{
		dc_printf( "The status is %d.\n", This_var );
	}
*/

class debug_command {
	public:
	char *name;
	char *help;
	void (*func)();
	debug_command(char *name,char *help,void (*func)());	// constructor
};

#define DCF(function_name,help_text)			\
		void dcf_##function_name();	\
		debug_command dc_##function_name(#function_name,help_text,dcf_##function_name);	\
		void dcf_##function_name()		

// Starts the debug console
extern void debug_console( void (*func)() = NULL );

// The next three variables tell your function what to do.  It should
// only change something if the dc_command is set.   A minimal function
// needs to process the dc_command.   Usually, these will be called in
// these combinations:
// dc_command=true, dc_status=true  means process it and show status
// dc_help=true means show help only
// dc_status=true means show status only
// I would recommend doing this in each function:
// if (dc_command) { process command }
// if (dc_help) { print out help }
// if (dc_status) { print out status }
// with the last two being optional

extern int Dc_command;	// If this is set, then process the command
extern int Dc_help;		// If this is set, then print out the help text in the form, "usage: ... \nLong description\n" );
extern int Dc_status;		// If this is set, then print out the current status of the command.

void dc_get_arg(uint flags);		// Gets the next argument.   If it doesn't match the flags, this function will print an error and not return.
extern char *Dc_arg;		// The (lowercased) string value of the argument retrieved from dc_arg
extern char *Dc_arg_org;	// Dc_arg before it got converted to lowercase
extern uint Dc_arg_type;	// The type of dc_arg.
extern char *Dc_command_line;		// The rest of the command line, from the end of the last processed arg on.
extern int Dc_arg_int;		// If Dc_arg_type & ARG_INT or ARG_HEX is set, then this is the value
extern float Dc_arg_float;	// If Dc_arg_type & ARG_FLOAT is set, then this is the value

// Outputs text to the console
void dc_printf( char *format, ... );

// Each dc_arg_type can have one or more of these flags set.
// This is because some things can fit into two catagories.
// Like 1 can be an integer, a float, a string, or a true boolean
// value.
#define ARG_NONE		(1<<0)	// no argument
#define ARG_ANY		0xFFFFFFFF	// Anything.
#define ARG_STRING	(1<<1)	// any valid string
#define ARG_QUOTE		(1<<2)	// a quoted string
#define ARG_INT		(1<<3)	// a valid integer
#define ARG_FLOAT		(1<<4)	// a valid floating point number

// some specific commonly used predefined types. Can add up to (1<<31)
#define ARG_HEX		(1<<5)	// a valid hexadecimal integer. Note that ARG_INT will always be set also in this case.
#define ARG_TRUE		(1<<6)	// on, true, non-zero number
#define ARG_FALSE		(1<<7)	// off, false, zero
#define ARG_PLUS		(1<<8)	// Plus sign
#define ARG_MINUS		(1<<9)	// Minus sign
#define ARG_COMMA		(1<<10)	// a comma

// A shortcut for boolean only variables.
// Example:  
// DCF_BOOL( lighting, Show_lighting )
//
#define DCF_BOOL( function_name, bool_variable )	\
	void dcf_##function_name();	\
	debug_command dc_##function_name(#function_name,"Toggles "#bool_variable,dcf_##function_name );	\
	void dcf_##function_name()	{	\
	if ( Dc_command )	{	\
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		\
		if ( Dc_arg_type & ARG_TRUE )	bool_variable = 1;	\
		else if ( Dc_arg_type & ARG_FALSE ) bool_variable = 0;	\
		else if ( Dc_arg_type & ARG_NONE ) bool_variable ^= 1;	\
	}	\
	if ( Dc_help )	dc_printf( "Usage: %s [bool]\nSets %s to true or false.  If nothing passed, then toggles it.\n", #function_name, #bool_variable );	\
	if ( Dc_status )	dc_printf( "%s is %s\n", #function_name, (bool_variable?"TRUE":"FALSE") );	\
}


//======================================================================================
//======================================================================================
//======================================================================================



#include "math/fix.h"
#include "math/floating.h"

// Some constants for stuff
#define MAX_FILENAME_LEN	32			// Length for filenames, ie "title.pcx"
#define MAX_PATH_LEN			128		// Length for pathnames, ie "c:\bitmaps\title.pcx"

// contants and defined for byteswapping routines (useful for mac)

#define SWAPSHORT(x)	(							\
						((ubyte)x << 8) |					\
						(((ushort)x) >> 8)			\
						)
						
#define SWAPINT(x)		(							\
						(x << 24) |					\
						(((ulong)x) >> 24) |		\
						((x & 0x0000ff00) << 8) |	\
						((x & 0x00ff0000) >> 8)		\
						)

#ifndef MACINTOSH
#define INTEL_INT(x)	x
#define INTEL_SHORT(x)	x
#else
#define INTEL_INT(x)	SWAPINT(x)
#define INTEL_SHORT(x)	SWAPSHORT(x)
#endif

#define TRUE	1
#define FALSE	0

int myrand();

#define CLAMP(lValue,lMin,lMax) (min(max((lValue), (lMin)), (lMax)))

// Callback Loading function. 
// Pass NULL to turn it off.
// Call this with the name of a function.  That function will
// then get called in a different thread as the game is loading
extern int game_busy_callback( void (*callback)(void));

//=========================================================
// Functions to monitor performance
#ifndef NDEBUG

class monitor {
	public:
	char	*name;			
	int	value;			// Value that gets cleared to 0 each frame.
	int	min, max, sum, cnt;		// Min & Max of value.  Sum is used to calculate average 
	monitor(char *name);	// constructor
};

// Creates a monitor variable
#define MONITOR(function_name)				monitor mon_##function_name(#function_name)

// Increments a monitor variable
#define MONITOR_INC(function_name,inc)		do { mon_##function_name.value+=(inc); } while(0)

// Call this once per frame to update monitor file
void monitor_update();

#else

#define MONITOR(function_name)

#define MONITOR_INC(function_name,inc)		do { } while(0)

// Call this once per frame to update monitor file
#define monitor_update() do { } while(0)

#endif

#define NOX(s) s

char *XSTR(char *str, int index);

// Caps V between MN and MX.
template <class T> void CAP( T& v, T mn, T mx )
{
	if ( v < mn )	{
		v = mn;
	} else if ( v > mx )	{
		v = mx;
	}
}

// ========================================================
// stamp checksum stuff
// ========================================================

// here is the define for the stamp for this set of code
#define STAMP_STRING "\001\001\001\001\002\002\002\002Read the Foundation Novels from Asimov.  I liked them." 
#define STAMP_STRING_LENGTH	80
#define DEFAULT_CHECKSUM_STRING		"\001\001\001\001"
#define DEFAULT_TIME_STRING			"\002\002\002\002"

// macro to calculate the checksum for the stamp.  Put here so that we can use different methods
// for different applications.  Requires the local variable 'checksum' to be defined!!!
#define CALCULATE_STAMP_CHECKSUM() do {	\
		int i, found;	\
							\
		checksum = 0;	\
		for ( i = 0; i < (int)strlen(ptr); i++ ) {	\
			found = 0;	\
			checksum += ptr[i];	\
			if ( checksum & 0x01 )	\
				found = 1;	\
			checksum = checksum >> 1;	\
			if (found)	\
				checksum |= 0x80000000;	\
		}	\
		checksum |= 0x80000000;	\
	} while (0) ;

//=========================================================
// Memory management functions
//=========================================================

void vm_strdup_free(void *ptr);
void vm_strdup_free();

#ifndef NDEBUG
	// Debug versions

	// Returns 0 if not enough RAM.
	int vm_init(int min_heap_size);

	// Allocates some RAM.
	void *vm_malloc( int size, char *filename=NULL, int line=-1 );

	// 
	char *vm_strdup( const char *ptr, char *filename, int line );

	// Frees some RAM. 
	void vm_free( void *ptr, char *filename=NULL, int line=-1 );

	// Frees all RAM.
	void vm_free_all();

	// Easy to use macros
	#define VM_MALLOC(size) vm_malloc((size),__FILE__,__LINE__)
	#define VM_FREE(ptr) vm_free((ptr),__FILE__,__LINE__)

	#define malloc(size) vm_malloc((size),__FILE__,__LINE__)
	#define free(ptr) vm_free((ptr),__FILE__,__LINE__)
	#define strdup(ptr) vm_strdup((ptr),__FILE__,__LINE__)
	
#else
	// Release versions

	// Returns 0 if not enough RAM.
	int vm_init(int min_heap_size);

	// Allocates some RAM.
	void *vm_malloc( int size );

	// 
	char *vm_strdup( const char *ptr );

	// Frees some RAM. 
	void vm_free( void *ptr );

	// Frees all RAM.
	void vm_free_all();

	// Easy to use macros
	#define VM_MALLOC(size) vm_malloc(size)
	#define VM_FREE(ptr) vm_free(ptr)

	#define malloc(size) vm_malloc(size)
	#define free(ptr) vm_free(ptr)
	#define strdup(ptr) vm_strdup(ptr)
#endif


#endif		// PS_TYPES_H
