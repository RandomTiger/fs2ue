#pragma once

#ifndef UNITY_BUILD
#include "GlobalIncs/PsTypes.h"
#include "ComponentSystem/ComponentSystem.h"
#endif


struct sBigExplFlash
{
	sBigExplFlash()
		: max_flash_intensity(0)
		, cur_flash_intensity(0)
		, flash_start(0) {}

	float max_flash_intensity;	// max intensity
	float cur_flash_intensity;	// cur intensity
	int	flash_start;		// start time
};

class MsgScreenEffectsUpdate : public MessageBase
{
	MESSAGE_CLASS()
public:
	MsgScreenEffectsUpdate(const float lfDetla) : mfDelta(lfDetla) {}
	float mfDelta;

	friend class ScreenEffects;
};

class MsgFlash : public MessageBase
{
	MESSAGE_CLASS()
public:
	MsgFlash(const float lfFlashValue) 
		: r(lfFlashValue)
		, g(lfFlashValue)
		, b(lfFlashValue) 
	{}
	MsgFlash(const float lfFlashR, const float lfFlashG, const float lfFlashB) 
		: r(lfFlashR)
		, g(lfFlashG)
		, b(lfFlashB) 
	{}

private:
	float r, g, b;

	friend class ScreenEffects;
};


class MsgSetSunDrawCount : public MessageBase
{
	MESSAGE_CLASS()
public:
	MsgSetSunDrawCount(const int lCount) : mCount(lCount) {}
private:
	int mCount;
	friend class ScreenEffects;
};

class MsgShudderGetAngles : public MessageBase
{
	MESSAGE_CLASS()
public:
	MsgShudderGetAngles() 
	{
		mAngles.b = mAngles.h = mAngles.p = 0.0f;
	}

	// todo fix this
	ScreenEffects *mPtr;
	angles mAngles;

	angles *GetAngles() {return &mAngles;}
private:
	friend class ScreenEffects;
};

class MsgShudderApply : public MessageBase
{
	MESSAGE_CLASS()
public:
	MsgShudderApply(int lTime, float lIntensity) : mTime(lTime), mIntensity(lIntensity) {}
private:
	int mTime;
	float mIntensity;

	friend class ScreenEffects;
};

class ScreenEffects : public ComponentBase
{
public:
	ScreenEffects();

	bool ProcessMessage(MessageBase *lMsg);

	void game_flash_reset();

	// Adds a flash effect.  These can be positive or negative.
	// The range will get capped at around -1 to 1, so stick 
	// with a range like that.
	void game_flash( float r, float g, float b );

	// Adds a flash for Big Ship explosions
	// cap range from 0 to 1
	void big_explosion_flash(float flash);

	void game_flash_diminish(float frametime);

	void game_sunspot_process(float frametime);

	void game_shudder_apply(int time, float intensity);

	void SetGameShudderTime(const int lValue) {mGameShudderTime = lValue;}
	int  GetGameShudderTime() {return mGameShudderTime;}
	float GetGameShudderIntensity() {return mGameShudderIntensity;}
	int  GetGameShudderTotal() {return mGameShudderTotal;}

	void SetSunDrawCount(const int lValue) {mSunDrew = lValue;}
	int  GetSunDrawCount() {return mSunDrew;}
private:
	// game flash stuff
	float mfGameFlashRed;
	float mfGameFlashGreen;
	float mfGameFlashBlue;
	float mSunSpot;
	sBigExplFlash mBigExplFlash;

	// game shudder stuff (in ms)
	int mGameShudderTime;
	int mGameShudderTotal;

	// should be between 0.0 and 100.0
	float mGameShudderIntensity;	

	float mSupernovaLastGlare;
	int mSunDrew;

};