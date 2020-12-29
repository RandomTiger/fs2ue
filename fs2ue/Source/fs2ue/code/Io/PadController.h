/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __XBOX360PAD__
#define __XBOX360PAD__

#ifndef UNITY_BUILD
#include "TomLib/src/ComponentSystem/ComponentSystem.h"
#include "ControlsConfig.h"
#endif

enum TargetState
{
	TARGET_NORMAL,
	TARGET_SUBSYSTEM,
	TARGET_TURRET,
	TARGET_MAX,
};

class GameController;

class MsgGetPad : public MessageBase
{
	MESSAGE_CLASS()
public:
	MsgGetPad() : mpPad(0) {}
	GameController *GetPad() {return mpPad;}
private:
	GameController *mpPad;
	friend class GameController;
};

class GameController : public ComponentBase
{
public:
	GameController() 
		: m_TargetState(TARGET_NORMAL)
		, m_BackButtonDownTime(0.0f)
	{

	}
private:
	const static int DEFAULT_ANALOG_POWER_FACTOR = 5;

	int m_driverPad, m_gunnerPad;
	bool m_inputActions[CCFG_MAX];

	int m_TargetState;
	float m_BackButtonDownTime;

public:

	const static int ANALOG_MAX_MOVE = 32768;

	void ClearInputControl();
	bool GetInputControl(int type);

	// call once to init the mouse
	void Init(int driverPad, int gunnerPad);
	void Update();
	void UpdateGameControls(const float frameTimeSecs);

	bool HasUnpauseBeenPressed();

	bool IsAttached(const int pad);

	bool IsButtonDown(const int pad, const int button);
	bool IsButtonDownSinceLast(const int pad, const int button);

	void GetPadAnalogStickPos(const int pad, int &x, int &y, const bool left = true, const int power = DEFAULT_ANALOG_POWER_FACTOR);

	int  GetDriverPad() {return m_driverPad;}
	bool IsActive() { return IsAttached(0);}

	bool ProcessMessage(MessageBase *lpMsg);
};

GameController *GetPrimaryPad();
GameController *GetSecondryPad();

#endif
