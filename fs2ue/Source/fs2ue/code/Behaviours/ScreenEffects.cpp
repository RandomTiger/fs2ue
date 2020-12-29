// Backwards compatibility for non unity build
#ifndef UNITY_BUILD
#include "ScreenEffects.h"
#include "Io/Timer.h"
#include "Graphics/2d.h"
#include "render/3d.h"
#include "Starfield/Supernova.h"
#include "Starfield/Starfield.h"
#include "Lighting/Lighting.h"
#include "Object/FSObject.h"
#include "Ship/ShipFx.h"

//template <class T> T min(const T& a, const T& b) {return (a < b) ? a : b; }
//template <class T> T max(const T& a, const T& b) {return (a > b) ? a : b; }

#else
#include "UnityBuild.h"
#endif
//	Amount to diminish palette towards normal, per second.
const float DIMINISH_RATE = 0.75f;
const float	SUN_DIMINISH_RATE = 6.00f;
const float sn_glare_scale = 1.7f;

int MsgFlash::mMessageID = MessageBase::GetNewMessageID();
int MsgShudderApply::mMessageID = MessageBase::GetNewMessageID();
int MsgScreenEffectsUpdate::mMessageID = MessageBase::GetNewMessageID();
int MsgShudderGetAngles::mMessageID = MessageBase::GetNewMessageID();
int MsgSetSunDrawCount::mMessageID = MessageBase::GetNewMessageID();

ScreenEffects::ScreenEffects() 
: mGameShudderTime(-1)
, mGameShudderTotal(0)
, mGameShudderIntensity(0.0f)
, mSupernovaLastGlare(0.0f)
, mSunDrew(0)
{ 
	game_flash_reset(); 
}

// Resets the flash
void ScreenEffects::game_flash_reset()
{
	mfGameFlashRed = 0.0f;
	mfGameFlashGreen = 0.0f;
	mfGameFlashBlue = 0.0f;
	mSunSpot = 0.0f;
	mBigExplFlash.max_flash_intensity = 0.0f;
	mBigExplFlash.cur_flash_intensity = 0.0f;
	mBigExplFlash.flash_start = 0;
}


// Adds a flash effect.  These can be positive or negative.
// The range will get capped at around -1 to 1, so stick 
// with a range like that.
void ScreenEffects::game_flash( float r, float g, float b )
{
	mfGameFlashRed += r;
	mfGameFlashGreen += g;
	mfGameFlashBlue += b;

	if ( mfGameFlashRed < -1.0f )	{
		mfGameFlashRed = -1.0f;
	} else if ( mfGameFlashRed > 1.0f )	{
		mfGameFlashRed = 1.0f;
	}

	if ( mfGameFlashGreen < -1.0f )	{
		mfGameFlashGreen = -1.0f;
	} else if ( mfGameFlashGreen > 1.0f )	{
		mfGameFlashGreen = 1.0f;
	}

	if ( mfGameFlashBlue < -1.0f )	{
		mfGameFlashBlue = -1.0f;
	} else if ( mfGameFlashBlue > 1.0f )	{
		mfGameFlashBlue = 1.0f;
	}

}

// Adds a flash for Big Ship explosions
// cap range from 0 to 1
void ScreenEffects::big_explosion_flash(float flash)
{
	mBigExplFlash.flash_start = timestamp(1);

	if (flash > 1.0f) {
		flash = 1.0f;
	} else if (flash < 0.0f) {
		flash = 0.0f;
	}

	mBigExplFlash.max_flash_intensity = flash;
	mBigExplFlash.cur_flash_intensity = 0.0f;
}


// Call once a frame to diminish the
// flash effect to 0.
void ScreenEffects::game_flash_diminish(float flFrametime)
{
	float dec_amount = flFrametime*DIMINISH_RATE;

	if ( mfGameFlashRed > 0.0f ) {
		mfGameFlashRed -= dec_amount;		
		if ( mfGameFlashRed < 0.0f )
			mfGameFlashRed = 0.0f;
	} else {
		mfGameFlashRed += dec_amount;		
		if ( mfGameFlashRed > 0.0f )
			mfGameFlashRed = 0.0f;
	} 

	if ( mfGameFlashGreen > 0.0f ) {
		mfGameFlashGreen -= dec_amount;		
		if ( mfGameFlashGreen < 0.0f )
			mfGameFlashGreen = 0.0f;
	} else {
		mfGameFlashGreen += dec_amount;		
		if ( mfGameFlashGreen > 0.0f )
			mfGameFlashGreen = 0.0f;
	} 

	if ( mfGameFlashBlue > 0.0f ) {
		mfGameFlashBlue -= dec_amount;		
		if ( mfGameFlashBlue < 0.0f )
			mfGameFlashBlue = 0.0f;
	} else {
		mfGameFlashBlue += dec_amount;		
		if ( mfGameFlashBlue > 0.0f )
			mfGameFlashBlue = 0.0f;
	} 

	// update big_explosion_cur_flash
	const int TIME_UP   = 1500;
	const int TIME_DOWN	= 2500;

	int duration = TIME_UP + TIME_DOWN;
	int time = timestamp_until(mBigExplFlash.flash_start);
	if (time > -duration) {
		time = -time;
		if (time < TIME_UP) {
			mBigExplFlash.cur_flash_intensity = mBigExplFlash.max_flash_intensity * time / (float) TIME_UP;
		} else {
			time -= TIME_UP;
			mBigExplFlash.cur_flash_intensity = mBigExplFlash.max_flash_intensity * ((float) TIME_DOWN - time) / (float) TIME_DOWN;
		}
	}

	// palette flash
	{
		int r,g,b;

		// Change the 200 to change the color range of colors.
		r = fl2i( mfGameFlashRed*128.0f );  
		g = fl2i( mfGameFlashGreen*128.0f );   
		b = fl2i( mfGameFlashBlue*128.0f );  

		if ( mSunSpot > 0.0f )	{
			r += fl2i(mSunSpot*128.0f);
			g += fl2i(mSunSpot*128.0f);
			b += fl2i(mSunSpot*128.0f);
		}

		if ( mBigExplFlash.cur_flash_intensity  > 0.0f ) {
			r += fl2i(mBigExplFlash.cur_flash_intensity*128.0f);
			g += fl2i(mBigExplFlash.cur_flash_intensity*128.0f);
			b += fl2i(mBigExplFlash.cur_flash_intensity*128.0f);
		}

		r = CLAMP(r, 0, 255);
		g = CLAMP(g, 0, 255);
		b = CLAMP(b, 0, 255);

		if ( (r!=0) || (g!=0) || (b!=0) ) {
			gr_flash( r, g, b );
		}
	}

}

void ScreenEffects::game_sunspot_process(float flFrametime)
{
	int n_lights, idx;
	int sn_stage;
	float Sun_spot_goal = 0.0f;

	// supernova
	sn_stage = supernova_active();
	if(sn_stage){		
		// sunspot differently based on supernova stage
		switch(sn_stage){
			// approaching. player still in control
		case 1:	
			{
				float pct;
				pct = (1.0f - (supernova_time_left() / SUPERNOVA_CUT_TIME));

				vector light_dir;				
				light_get_global_dir(&light_dir, 0);
				float dot;
				dot = vm_vec_dot( &light_dir, &Eye_matrix.fvec );

				if(dot >= 0.0f){
					// scale it some more
					dot = dot * (0.5f + (pct * 0.5f));
					dot += 0.05f;					

					Sun_spot_goal += (dot * sn_glare_scale);
				}

				// draw the sun glow
				if ( !shipfx_eye_in_shadow( &Eye_position, Viewer_obj, 0 ) )	{
					// draw the glow for this sun
					stars_draw_sun_glow(0);	
				}

				mSupernovaLastGlare = Sun_spot_goal;
				break;
			}

			// camera cut. player not in control. note : at this point camera starts out facing the sun. so we can go nice and bright
		case 2: 					
		case 3:
			Sun_spot_goal = 0.9f;
			Sun_spot_goal += (1.0f - (supernova_time_left() / SUPERNOVA_CUT_TIME)) * 0.1f;

			if(Sun_spot_goal > 1.0f){
				Sun_spot_goal = 1.0f;
			}

			Sun_spot_goal *= sn_glare_scale;
			mSupernovaLastGlare = Sun_spot_goal;
			break;		

			// fade to white. display dead popup
		case 4:
		case 5:
			mSupernovaLastGlare += (2.0f * flFrametime);
			if(mSupernovaLastGlare > 2.0f){
				mSupernovaLastGlare = 2.0f;
			}

			Sun_spot_goal = mSupernovaLastGlare;
			break;
		}

		mSunDrew = 0;				
	} else {
		if ( mSunDrew )	{
			// check sunspots for all suns
			n_lights = light_get_global_count();

			// check
			for(idx=0; idx<n_lights; idx++){
				//(vector *eye_pos, matrix *eye_orient)
				if ( !shipfx_eye_in_shadow( &Eye_position, Viewer_obj, idx ) )	{

					vector light_dir;				
					light_get_global_dir(&light_dir, idx);

					float dot = vm_vec_dot( &light_dir, &Eye_matrix.fvec )*0.5f+0.5f;

					Sun_spot_goal += (float)pow(dot,85.0f);

					// draw the glow for this sun
					stars_draw_sun_glow(idx);				
				} else {
					Sun_spot_goal = 0.0f;
				}
			}

			mSunDrew = 0;
		} else {
			Sun_spot_goal = 0.0f;
		}
	}

	float dec_amount = flFrametime*SUN_DIMINISH_RATE;

	if ( mSunSpot < Sun_spot_goal )	{
		mSunSpot += dec_amount;
		if ( mSunSpot > Sun_spot_goal )	{
			mSunSpot = Sun_spot_goal;
		}
	} else if ( mSunSpot > Sun_spot_goal )	{
		mSunSpot -= dec_amount;
		if ( mSunSpot < Sun_spot_goal )	{
			mSunSpot = Sun_spot_goal;
		}
	}
}

void ScreenEffects::game_shudder_apply(int time, float intensity)
{
	mGameShudderTime = timestamp(time);
	mGameShudderTotal = time;
	mGameShudderIntensity = intensity;
}

bool ScreenEffects::ProcessMessage(MessageBase *lpMsgBase)
{
	if(lpMsgBase->GetMessageID() == MsgFlash::GetStaticMessageID())
	{
		MsgFlash *lMsg = (MsgFlash *) lpMsgBase;
		game_flash(lMsg->r, lMsg->g, lMsg->b);
		return true;
	}

	if(lpMsgBase->GetMessageID() == MsgShudderApply::GetStaticMessageID())
	{
		MsgShudderApply *lMsg = (MsgShudderApply *) lpMsgBase;
		game_shudder_apply(lMsg->mTime, lMsg->mIntensity);
		return true;
	}

	if(lpMsgBase->GetMessageID() == MsgScreenEffectsUpdate::GetStaticMessageID())
	{
		MsgScreenEffectsUpdate *lMsg = (MsgScreenEffectsUpdate *) lpMsgBase;
		
		game_sunspot_process(lMsg->mfDelta);
		game_flash_diminish(lMsg->mfDelta);
		return true;
	}

	if(lpMsgBase->GetMessageID() == MsgSetSunDrawCount::GetStaticMessageID())
	{
		MsgSetSunDrawCount *lMsg = (MsgSetSunDrawCount *) lpMsgBase;

		SetSunDrawCount(lMsg->mCount);
		return true;
	}

	if(lpMsgBase->GetMessageID() == MsgShudderGetAngles::GetStaticMessageID())
	{
		MsgShudderGetAngles *lMsg = (MsgShudderGetAngles *) lpMsgBase;

		if(GetGameShudderTime() != -1){
			// if the timestamp has elapsed
			if(timestamp_elapsed(GetGameShudderTime())){
				SetGameShudderTime(-1);
			} 
			// otherwise apply some shudder
			else {
				const float dtime = (float) timestamp_until(GetGameShudderTime());

				int r1 = myrand();
				int r2 = myrand();
				const float factor = 0.5f - fl_abs(0.5f - dtime/(float) GetGameShudderTotal());
				lMsg->mAngles.p += (GetGameShudderIntensity() / 200.0f) * (float) (r1-RAND_MAX/2)/RAND_MAX * factor;
				lMsg->mAngles.h += (GetGameShudderIntensity() / 200.0f) * (float) (r2-RAND_MAX/2)/RAND_MAX * factor;
			}
		}
		return true;
	}

	return false;
}