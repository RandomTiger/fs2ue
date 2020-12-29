#ifndef UNITY_BUILD
#include "PadController.h"

#include <windows.h>
#include <xinput.h>
#include "MouseController.h"
#include "InputController.h"
#include "Mouse.h"

#include "keycontrol.h"
#endif

class Pad
{
public:
	const static int MAX_CONTROLLERS = 4;

	XINPUT_STATE m_state;
	XINPUT_STATE m_previousState;

	bool m_bConnected;

	inline bool GetButtonState(const int button) 
	{
		return (m_state.Gamepad.wButtons & button) == button;
	}
};

int MsgGetPad::mMessageID = MessageBase::GetNewMessageID();

GameController g_PadController[Pad::MAX_CONTROLLERS];

GameController *GetPrimaryPad() {return &(g_PadController[0]);}
GameController *GetSecondryPad() {return &(g_PadController[1]);}

Pad g_PadControllers[Pad::MAX_CONTROLLERS];

void GameController::Init(int driverPad, int gunnerPad)
{
	m_driverPad = driverPad;
	m_gunnerPad = gunnerPad;
}

bool GameController::IsAttached(const int pad)
{
	return g_PadControllers[pad].m_bConnected;
}

bool GameController::IsButtonDown(const int pad, const int button)
{
	return (g_PadControllers[pad].m_state.Gamepad.wButtons & button) == button;
}

bool GameController::IsButtonDownSinceLast(const int pad, const int button)
{
	return (g_PadControllers[pad].m_state.Gamepad.wButtons & button) == button && (g_PadControllers[pad].m_previousState.Gamepad.wButtons & button) == 0;
}

void GameController::GetPadAnalogStickPos(const int pad, int &x, int &y, const bool left, const int power)
{
	SHORT thumpstickX = g_PadControllers[pad].m_state.Gamepad.sThumbLX;
	SHORT thumpstickY = g_PadControllers[pad].m_state.Gamepad.sThumbLY;
	if(left == false)
	{
		thumpstickX = g_PadControllers[pad].m_state.Gamepad.sThumbRX;
		thumpstickY = g_PadControllers[pad].m_state.Gamepad.sThumbRY;
	}

	const float axisX = ((float) thumpstickX) / (float) ANALOG_MAX_MOVE;
	const float resultX = pow(axisX, power);
	x = (int) (resultX * ANALOG_MAX_MOVE);

	const float axisY = ((float) thumpstickY) / (float) ANALOG_MAX_MOVE;
	const float resultY = pow(axisY, power);
	y = (int) (resultY * ANALOG_MAX_MOVE);
}

bool GameController::GetInputControl(int type)
{
	return m_inputActions[type];
}

void GameController::ClearInputControl()
{
	memset(m_inputActions, 0, sizeof(m_inputActions));
}

void GameController::UpdateGameControls(const float frameTimeSecs)
{
	const float SHOULDER_DEADZONE = ( 0.24f * 255.0f ) ;

	// Targetting
	if(m_TargetState == TARGET_NORMAL)
	{	
		m_inputActions[TARGET_NEXT_CLOSEST_HOSTILE] = IsButtonDownSinceLast(m_gunnerPad, XINPUT_GAMEPAD_RIGHT_SHOULDER);
		m_inputActions[TARGET_PREV_CLOSEST_HOSTILE] = IsButtonDownSinceLast(m_gunnerPad, XINPUT_GAMEPAD_LEFT_SHOULDER);
	} 
	else if(m_TargetState == TARGET_SUBSYSTEM)
	{
		m_inputActions[TARGET_NEXT_SUBOBJECT] = IsButtonDownSinceLast(m_gunnerPad, XINPUT_GAMEPAD_RIGHT_SHOULDER);
		m_inputActions[TARGET_PREV_SUBOBJECT] = IsButtonDownSinceLast(m_gunnerPad, XINPUT_GAMEPAD_LEFT_SHOULDER);
	} 
	else if(m_TargetState == TARGET_TURRET)
	{
		m_inputActions[TARGET_NEXT_LIVE_TURRET] = IsButtonDownSinceLast(m_gunnerPad, XINPUT_GAMEPAD_RIGHT_SHOULDER);
		m_inputActions[TARGET_PREV_LIVE_TURRET] = IsButtonDownSinceLast(m_gunnerPad, XINPUT_GAMEPAD_LEFT_SHOULDER);
	}

	m_inputActions[TARGET_SHIP_IN_RETICLE]       = IsButtonDownSinceLast(m_gunnerPad, XINPUT_GAMEPAD_Y);

	if(IsButtonDownSinceLast(m_gunnerPad, XINPUT_GAMEPAD_X))
	{
		m_TargetState = (m_TargetState + 1) % TARGET_MAX;

		if(m_TargetState == TARGET_NORMAL)
		{
			m_inputActions[STOP_TARGETING_SUBSYSTEM] = true;
		}
		else if(m_TargetState == TARGET_SUBSYSTEM)
		{
			m_inputActions[TARGET_NEXT_SUBOBJECT] = true;
		} 
		else if(m_TargetState == TARGET_TURRET)
		{
			m_inputActions[TARGET_NEXT_LIVE_TURRET] = true;
		}
	}
	else
	{
		m_inputActions[STOP_TARGETING_SUBSYSTEM] = false;
	}


	// Weapons
	m_inputActions[LAUNCH_COUNTERMEASURE] = IsButtonDownSinceLast(m_gunnerPad, XINPUT_GAMEPAD_A);
	m_inputActions[FIRE_SECONDARY]        = IsButtonDown(m_gunnerPad, XINPUT_GAMEPAD_B);
	m_inputActions[FIRE_PRIMARY] = (g_PadControllers[m_gunnerPad].m_state.Gamepad.bRightTrigger > SHOULDER_DEADZONE);

	// Speed
	m_inputActions[AFTERBURNER] = (g_PadControllers[m_driverPad].m_state.Gamepad.bLeftTrigger > SHOULDER_DEADZONE);
	m_inputActions[TOGGLE_AUTO_MATCH_TARGET_SPEED] = IsButtonDownSinceLast(m_driverPad, XINPUT_GAMEPAD_LEFT_THUMB);
	m_inputActions[ZERO_THROTTLE] = IsButtonDownSinceLast(m_driverPad, XINPUT_GAMEPAD_RIGHT_THUMB);

	
	// shields
	m_inputActions[SHIELD_XFER_TOP]       = IsButtonDown(m_gunnerPad, XINPUT_GAMEPAD_DPAD_UP);
	m_inputActions[SHIELD_XFER_BOTTOM]    = IsButtonDown(m_gunnerPad, XINPUT_GAMEPAD_DPAD_DOWN);
	m_inputActions[SHIELD_XFER_RIGHT]     = IsButtonDown(m_gunnerPad, XINPUT_GAMEPAD_DPAD_RIGHT); 
	m_inputActions[SHIELD_XFER_LEFT]      = IsButtonDown(m_gunnerPad, XINPUT_GAMEPAD_DPAD_LEFT);

	if(IsButtonDownSinceLast(m_driverPad, XINPUT_GAMEPAD_START) || IsButtonDownSinceLast(m_gunnerPad, XINPUT_GAMEPAD_START))
	{
		game_process_pause_key();
	}
		
	if(IsButtonDown(m_driverPad, XINPUT_GAMEPAD_BACK))
	{
		m_BackButtonDownTime += frameTimeSecs;
	}
	else
	{
		m_BackButtonDownTime = 0;
	}

	const float kHoldTimeForWrapOut = 1.5f;
	if(m_BackButtonDownTime > kHoldTimeForWrapOut)
	{
		m_inputActions[END_MISSION] = true;
		m_BackButtonDownTime = 0;
	}
	else if(IsButtonDownSinceLast(m_gunnerPad, XINPUT_GAMEPAD_BACK) || IsButtonDownSinceLast(m_driverPad, XINPUT_GAMEPAD_BACK))
	{
		// Go to new pause screen
	}
	
	//int roll, engine;
	//GetPadAnalogStickPos(m_driverPad, roll, engine, false);
}

bool GameController::HasUnpauseBeenPressed()
{
	return (IsButtonDownSinceLast(m_driverPad, XINPUT_GAMEPAD_START) || IsButtonDownSinceLast(m_gunnerPad, XINPUT_GAMEPAD_START));
}

void GameController::Update()
{
#if !defined(FS2_UE)

	// Default to 24% of the +/- 32767 range.   This is a reasonable default value but can be altered if needed.
	const float INPUT_DEADZONE = ( 0.24f * FLOAT(0x7FFF) ) ;
	const bool   g_bDeadZoneOn = true;

	DWORD dwResult;
	for( DWORD i = 0; i < Pad::MAX_CONTROLLERS; i++ )
	{
		memcpy(&g_PadControllers[i].m_previousState, &g_PadControllers[i].m_state, sizeof(XINPUT_STATE));
		dwResult = XInputGetState( i, &g_PadControllers[i].m_state );
		g_PadControllers[i].m_bConnected = ( dwResult == ERROR_SUCCESS );	
		if( g_PadControllers[i].m_bConnected )
		{
			if( g_bDeadZoneOn )
			{
				// Zero value if thumbsticks are within the dead zone 
				if( ( g_PadControllers[i].m_state.Gamepad.sThumbLX < INPUT_DEADZONE &&
					  g_PadControllers[i].m_state.Gamepad.sThumbLX > -INPUT_DEADZONE ) &&
					( g_PadControllers[i].m_state.Gamepad.sThumbLY < INPUT_DEADZONE &&
					  g_PadControllers[i].m_state.Gamepad.sThumbLY > -INPUT_DEADZONE ) )
				{
					g_PadControllers[i].m_state.Gamepad.sThumbLX = 0;
					g_PadControllers[i].m_state.Gamepad.sThumbLY = 0;
				}

				if( ( g_PadControllers[i].m_state.Gamepad.sThumbRX < INPUT_DEADZONE &&
					  g_PadControllers[i].m_state.Gamepad.sThumbRX > -INPUT_DEADZONE ) &&
					( g_PadControllers[i].m_state.Gamepad.sThumbRY < INPUT_DEADZONE &&
					  g_PadControllers[i].m_state.Gamepad.sThumbRY > -INPUT_DEADZONE ) )
				{
					g_PadControllers[i].m_state.Gamepad.sThumbRX = 0;
					g_PadControllers[i].m_state.Gamepad.sThumbRY = 0;
				}
			}
		}
	}
#endif
}

bool GameController::ProcessMessage(MessageBase *lpMsgBase)
{
	if(lpMsgBase->GetMessageID() == MsgGetPad::GetStaticMessageID())
	{
		MsgGetPad *lMsg = (MsgGetPad *) lpMsgBase;
		lMsg->mpPad = this;
		return true;
	}
	
	return false;
}