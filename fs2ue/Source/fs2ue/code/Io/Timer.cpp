/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#ifndef UNITY_BUILD
#include "Timer.h"

#include <windows.h>

#include "limits.h"
#include "2d.h"
#include "Alphacolors.h"
#endif

enum TimeMethod
{
	METHOD_QUERY_PREFORMANCE,
	METHOD_GET_TIME,
};

const TimeMethod g_TimeMethod = METHOD_GET_TIME;

#ifndef NDEBUG
	#define USE_TIMING
#endif

static __int64 Timer_last_value, Timer_base;
__int64 freq;

static int Timer_inited = 0;

static CRITICAL_SECTION Timer_lock;

void timer_close()
{
	if ( Timer_inited )	{
		Timer_inited = 0;
		DeleteCriticalSection( &Timer_lock );
	}
}

void timer_init()
{
	if ( !Timer_inited )	{

		InitializeCriticalSection(&Timer_lock);
		Timer_inited = 1;
		atexit(timer_close);

		if(g_TimeMethod == METHOD_QUERY_PREFORMANCE)
		{
			QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

			QueryPerformanceCounter((LARGE_INTEGER*)&Timer_base);
			QueryPerformanceCounter((LARGE_INTEGER*)&Timer_last_value);

			// Fix to keep XP happy when running on a duel core
			// May need to revist the whole timing code
			DWORD_PTR mask = 1; 
			SetThreadAffinityMask(GetCurrentThread(), mask);
		}
		else if(g_TimeMethod == METHOD_GET_TIME)
		{
			Timer_base = Timer_last_value = timeGetTime();
		}
	}
}

static int timer_get()
{
	__int64 time_now;
	if(g_TimeMethod == METHOD_QUERY_PREFORMANCE)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&time_now);
	}
	else if(g_TimeMethod == METHOD_GET_TIME)
	{
		time_now =  timeGetTime();
	}

	if ( time_now < Timer_last_value ) {
		// the clock has rolled!
		Timer_base = time_now;
		mprintf(("TIMER ROLLED!\n"));
	}

	Timer_last_value = time_now;

	if(g_TimeMethod == METHOD_QUERY_PREFORMANCE)
	{
		uint result = (uint) (((time_now - Timer_base) * 1000) / freq);
		printf("Time: %d\n",result);
		return result;
	}
	else if(g_TimeMethod == METHOD_GET_TIME)
	{
		return time_now - Timer_base;
	}

	return -1;
}

fix timer_get_fixed_seconds()
{
	if (!Timer_inited) {
		Int3();					// Make sure you call timer_init before anything that uses timer functions!
		return 0;
	}

	longlong time = timer_get();
	time *= 65536;

#if _DEBUG
	if(time < 0)
	{
		Int3();	
	}
#endif
	return (fix) (time / 1000);
}


float timer_get_seconds_float()
{
	if (!Timer_inited) {
		Int3();					// Make sure you call timer_init before anything that uses timer functions!
		return 0;
	}

	longlong time = timer_get();


	Assert(time > 0);
	return ((float) time) / 1000.0f;
}

int timer_get_milliseconds()
{

	return timer_get();
}

int timer_get_microseconds()
{
	return timer_get() * 1000;
}

int timer_get_seconds()
{
	return (int) timer_get_fixed_seconds(); 
}

// 0 means invalid,
// 1 means always return true
// 2 and above actually check the time
int timestamp_ticker = 2;

void timestamp_reset()
{
	timestamp_ticker = 2;
}

// Restrict all time values between 0 and MAX_TIME
// so we don't have to use UINTs to calculate rollover.
// For debugging & testing, you could set this to 
// something like 1 minute (6000).
#define MAX_TIME (INT_MAX/2)

void timestamp_inc(float frametime)
{
	timestamp_ticker += (int)(frametime*TIMESTAMP_FREQUENCY);

	if ( timestamp_ticker > MAX_TIME )	{
		timestamp_ticker = 2;		// Roll!
	}

	if (timestamp_ticker < 2 ) {
		mprintf(("Whoa!!!  timestamp_ticker < 2 -- resetting to 2!!!\n"));
		timestamp_ticker = 2;
	}
}

int timestamp(int delta_ms )
{
	int t2;
	if (delta_ms < 0 ) return 0;
	if (delta_ms == 0 ) return 1;
	t2 = timestamp_ticker + delta_ms;
	if ( t2 > MAX_TIME )	{
		// wrap!!!
		t2 = delta_ms - (MAX_TIME-timestamp_ticker);
	}
	if (t2 < 2 ) t2 = 2;	// hack??
	return t2;
}

//	Returns milliseconds until timestamp will elapse.
//	Negative value gives milliseconds ago that timestamp elapsed.
int timestamp_until(int stamp)
{
	// JAS: FIX
	// HACK!! This doesn't handle rollover!
	// (Will it ever happen?)
	
	return stamp - timestamp_ticker;

/*
	uint	delta;

	delta = stamp - timestamp_ticker;
	

	if (delta > UINT_MAX/2)
		delta = UINT_MAX - delta + 1;
	else if (delta < - ( (int) (UINT_MAX/2)))
		delta = UINT_MAX + delta + 1;

	return delta;
*/
}

// alternate timestamp functions.  The way these work is you call xtimestamp() to get the
// current counter value, and then call
int timestamp()
{
	return timestamp_ticker;
}

int timestamp_has_time_elapsed(int stamp, int time)
{
	int t;

	if (time <= 0)
		return 1;

	t = stamp + time;
	if (t <= timestamp_ticker)
		return 1;  // if we are unlucky enough to have it wrap on us, this will assume time has elapsed.

	return 0;
}

// timing functions -------------------------------------------------------------------------------

#define MAX_TIMING_EVENTS		15

// timing struct
#ifdef USE_TIMING

typedef struct timing {
	char name[64];
	int microseconds_total;
	int start;
	int ref_count;
} timing;

timing Timing_frame;
timing Timing_events[MAX_TIMING_EVENTS];
int Timing_event_count = 0;

#endif

// lookup a timing event
int timing_event_lookup(char *event_name)
{
#ifndef USE_TIMING
	return -1;
#else
	int idx;

	// sanity
	if(event_name == NULL){
		return -1;
	}

	// look through all events
	for(idx=0; idx<MAX_TIMING_EVENTS; idx++){
		if(!stricmp(Timing_events[idx].name, event_name)){
			return idx;
		}
	}

	return -1;
#endif
}

// start timing frame stuff
void timing_frame_start()
{
#ifndef USE_TIMING
	return;
#else
	int idx;

	// restart the frame
	Timing_event_count = 0;
	Timing_frame.start = timer_get_microseconds();
	for(idx=0; idx<MAX_TIMING_EVENTS; idx++){
		Timing_events[idx].microseconds_total = 0;
		strcpy(Timing_events[idx].name, "");
		Timing_events[idx].ref_count = 0;
	}
#endif
}

// done timing the frame
void timing_frame_stop()
{
#ifndef USE_TIMING
	return;
#else	
	int stop_time;

	// stop time
	stop_time = timer_get_microseconds();

	// calc times
	Timing_frame.microseconds_total = stop_time - Timing_frame.start;	
#endif
}

// get the total frame time in microseconds
int timing_frame_total()
{
#ifndef USE_TIMING
	return 0;
#else
	return Timing_frame.microseconds_total;
#endif
}

// time an individual event
void timing_event_start(char *event_name)
{
#ifndef USE_TIMING
	return;
#else
	int event;

	// sanity
	if(event_name == NULL){
		return;
	}

	// try and find the event
	event = timing_event_lookup(event_name);

	// if we already have one
	if(event != -1){
		Assert(Timing_events[event].ref_count == 0);
		Timing_events[event].start = timer_get_microseconds();
		Timing_events[event].ref_count++;
	}
	// if we need to add a new one
	else {
		if(Timing_event_count < MAX_TIMING_EVENTS){
			strcpy(Timing_events[Timing_event_count].name, event_name);
			Timing_events[Timing_event_count].start = timer_get_microseconds();
			Timing_events[Timing_event_count++].ref_count++;
		}
	}
#endif
}

// stop timing an event
void timing_event_stop(char *event_name)
{
#ifndef USE_TIMING
	return;
#else
	int event;

	// sanity
	if(event_name == NULL){
		return;
	}

	// try and find the event
	event = timing_event_lookup(event_name);

	// if we already have one
	if(event != -1){
		Assert(Timing_events[event].ref_count == 1);
		Timing_events[event].microseconds_total += timer_get_microseconds() - Timing_events[event].start;
		Timing_events[event].ref_count--;
	}
#endif
}

// get the total time for an event in microseconds
int timing_event_total(char *event_name)
{
#ifndef USE_TIMING
	return -1;
#else
	int event;

	// sanity
	if(event_name == NULL){
		return -1;
	}

	// try and find the event
	event = timing_event_lookup(event_name);
	if(event == -1){
		return -1;
	}
	
	return Timing_events[event].microseconds_total;
#endif
}

// get the percentage of total frametime for the event (0.0 to 1.0)
float timing_event_pct(char *event_name)
{
#ifndef USE_TIMING
	return 0.0f;
#else
	int event;

	// sanity
	if(event_name == NULL){
		return -1.0f;
	}

	// try and find the event
	event = timing_event_lookup(event_name);
	if(event == -1){
		return -1.0f;
	}

	return (float)((double)Timing_events[event].microseconds_total / (double)Timing_frame.microseconds_total);
#endif
}

// display timing 
void timing_display(int x, int y)
{
#ifndef USE_TIMING
	return;
#else
	int idx;

	gr_set_color_fast(&Color_bright_blue);

	// total time
	gr_printf(x, y, "Total time %f\n", (float)timing_frame_total() / 1000000.0f);
	y += 10;

	// each event percentage
	for(idx=0; idx<Timing_event_count; idx++){
		gr_printf(x, y, "%s : %f\n", Timing_events[idx].name, timing_event_pct(Timing_events[idx].name));
		y += 10;
	}
#endif
}
