/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifdef FS2_UE
#include "Game/Game.h"
#else
// Backwards compatibility for non unity build
#ifndef UNITY_BUILD
// Pretend we are a unity build
#define UNITY_BUILD
// Include ALL headers
#include "UnityBuild.h"
// Disable Pretend
#undef UNITY_BUILD
#endif
#endif

namespace Game
{
	int View_percent = 100;

}

int GameGetViewPercent()
{
	return Game::View_percent;
}

void GameSetViewPercent(const int liValue)
{
	Game::View_percent = liValue;
}

// Set the clip region for the 3d rendering window
void GameSetViewClip()
{
	using namespace Game;
	if ((Game_mode & GM_DEAD) || (supernova_active() >= 2))	{
		// Set the clip region for the letterbox "dead view"
		const int yborder = gr_screen.max_h/4;

		gr_set_clip(0, yborder, gr_screen.max_w, gr_screen.max_h - yborder*2 );	
	} else {
		// Set the clip region for normal view
		if ( View_percent >= 100 )	{
			gr_reset_clip_splitscreen_safe();
		} else {
			int xborder, yborder;

			if ( View_percent < 5 )	{
				View_percent = 5;
			}

			float fp = i2fl(View_percent)/100.0f;
			int fi = fl2i(fl_sqrt(fp)*100.0f);
			if ( fi > 100 ) fi=100;

			xborder = ( gr_screen.max_w*(100-fi) )/200;
			yborder = ( gr_screen.max_h*(100-fi) )/200;

			gr_set_clip(xborder, yborder, gr_screen.max_w-xborder*2,gr_screen.max_h-yborder*2 );
		}
	}
}