#ifndef UNITY_BUILD
#include "ForceFeedback.h"
#include "InputController.h"
#include "PadController.h"

#include <windows.h>
#include <xinput.h>
#include <timer.h>
#include "Playerman/Player.h"
#endif

ForceFeedback g_ForceFeedback;

struct CONTROLER_STATE
{
    XINPUT_STATE lastState;
    XINPUT_STATE state;
    DWORD dwResult;
    XINPUT_VIBRATION vibration;
};

CONTROLER_STATE g_Controllers[ForceFeedback::MAX_CONTROLLERS];


ForceFeedback::Effect::Effect() : 
	m_motor(0), 
	m_duration(0), 
	m_timeInPlay(0), 
	m_graph(LINEAR_DECREASE), 
	m_rumble(0), 
	m_peak(0)
{

}

void ForceFeedback::Effect::Start(const int motor, const int duration, const int graph, const int peak)
{
	m_motor      = motor;
	m_duration   = duration;
	m_active     = true;
	m_timeInPlay = 0;
	m_graph	     = graph;
	m_peak		 = peak;
}

void ForceFeedback::Effect::EndEffect()
{
	m_active = false;
}

void ForceFeedback::Effect::Update(float frameTimeMS)
{
	if(m_graph == LINEAR_DECREASE)
	{
		float changer = 1.0f - (m_timeInPlay) / (m_duration);
		m_rumble = m_peak + (int) (((float) m_peak) * changer);
	}
	else
	{
		assert(0);
	}

	m_timeInPlay += frameTimeMS;
	if(m_timeInPlay > m_duration && m_duration != -1)
	{
		m_active = false;
	}
}


void ForceFeedback::Init(int type)
{
	m_type = type;
	m_initType = type;
	joy_ff_init();
}

void ForceFeedback::Shutdown(int type)
{
	joy_ff_shutdown();
}

void ForceFeedback::StopEffects()
{
	if(m_type == InputController::kIN_360PAD)
	{
		for(int padNum = 0; padNum < MAX_CONTROLLERS; padNum++)
		{
			XINPUT_VIBRATION amount;
			amount.wLeftMotorSpeed  = 0;
			amount.wRightMotorSpeed = 0;
			g_Controllers[padNum].dwResult = XInputSetState( padNum, &amount );
		}
	}
	else
	{
		joy_ff_stop_effects();
	}
}

void ForceFeedback::MissionInit(vector v)
{
	joy_ff_mission_init(v);
}

void ForceFeedback::Reacquire()
{
	joy_reacquire_ff();
}

void ForceFeedback::Unacquire()
{
	joy_unacquire_ff();
}
// collided with other ship
void ForceFeedback::PlayVectorEffect(vector *v, float scaler)
{
	if(m_type == InputController::kIN_360PAD)
	{
		g_ForceFeedback.StartEffect(0, 0, Effect::LINEAR_DECREASE, ONE_SEC * 0.25, MAX_RUMBLE / 5);
	}
	else
	{
		joy_ff_play_vector_effect(v, scaler);
	}
}

// hit by missle
void ForceFeedback::PlayDirEffect(float x, float y)
{
	if(m_type == InputController::kIN_360PAD)
	{
		g_ForceFeedback.StartEffect(0, 1, Effect::LINEAR_DECREASE, ONE_SEC * 0.25, MAX_RUMBLE / 4);
	}
	else
	{
		joy_ff_play_dir_effect(x, y);
	}
}

void ForceFeedback::PlayPrimaryShoot(int gain)
{
	if(m_type == InputController::kIN_360PAD)
	{
		g_ForceFeedback.StartEffect(0, 0, Effect::LINEAR_DECREASE, ONE_SEC * 0.25, MAX_RUMBLE / 20);
	}
	else
	{
		joy_ff_play_primary_shoot(gain);
	}
}

void ForceFeedback::PlaySecondaryShoot(int gain)
{
	if(m_type == InputController::kIN_360PAD)
	{
		g_ForceFeedback.StartEffect(0, 1, Effect::LINEAR_DECREASE, ONE_SEC * 0.25, MAX_RUMBLE / 15);
	}
	else
	{
		joy_ff_play_secondary_shoot(gain);
	}
}

void ForceFeedback::AdjustHandling(int speed)
{
	joy_ff_adjust_handling(speed);
}

// Support ship has docked
void ForceFeedback::Docked()
{
	if(m_type == InputController::kIN_360PAD)
	{
		g_ForceFeedback.StartEffect(0, 1, Effect::LINEAR_DECREASE, ONE_SEC * 1, MAX_RUMBLE / 2);
	}
	else
	{
		joy_ff_docked();
	}
}

// Support ship is reloading
void ForceFeedback::PlayReloadEffect()
{
	if(m_type == InputController::kIN_360PAD)
	{	
		g_ForceFeedback.StartEffect(0, 0, Effect::LINEAR_DECREASE, ONE_SEC * 0.5, MAX_RUMBLE / 3);
	}
	else
	{
		joy_ff_play_reload_effect();
	}
}

void ForceFeedback::AfterburnOn()
{
	joy_ff_afterburn_on();
}

void ForceFeedback::AfterburnOff()
{
	joy_ff_afterburn_off();
}

// Player death
void ForceFeedback::Explode()
{
	if(m_type == InputController::kIN_360PAD)
	{
		g_ForceFeedback.StartEffect(0, 0, Effect::LINEAR_DECREASE, ONE_SEC * 3.0, MAX_RUMBLE);
	}
	else
	{
		joy_ff_explode();
	}
}

void ForceFeedback::FlyBy(int mag)
{
	if(m_type == InputController::kIN_360PAD)
	{
		g_ForceFeedback.StartEffect(0, 0, Effect::LINEAR_DECREASE, ONE_SEC * 1.0, MAX_RUMBLE / 20);
	}
	else
	{
		joy_ff_fly_by(mag); 
	}
}

// Deathroll before player dies
void ForceFeedback::Deathroll()
{
	if(m_type == InputController::kIN_360PAD)
	{
		g_ForceFeedback.StartEffect(0, 0, Effect::LINEAR_DECREASE, ONE_SEC * 3.0, MAX_RUMBLE);
	}
	else
	{
		joy_ff_deathroll();
	}
}

ForceFeedback::Effect &ForceFeedback::StartEffect(const int padNum, const int motor, const int graph, const int duration, const int peak)
{
	assert(padNum >= 0 && padNum < MAX_CONTROLLERS);

	int freeSlot         = -1;
	int lowestRumbleSlot = -1;
	int lowestRumble     = -1;
	for(int i = 0; i < MAX_EFFECTS_PER_CONTROLLER; i++)
	{
		if(m_Effects[padNum][i].IsActive() == false)
		{
			freeSlot = i;
			break;
		}

		const int rumble = m_Effects[padNum][i].GetRumble();
		if(rumble < lowestRumble || lowestRumble == -1)
		{
			lowestRumble     = rumble;
			lowestRumbleSlot = i;
		}
	}

	// Bring on the new effect at the expense of the least powerful effect
	// if no slots are left
	if(freeSlot == -1)
	{
		freeSlot = lowestRumbleSlot;
	}

	m_Effects[padNum][freeSlot].Start(motor, duration, graph, peak);
	return m_Effects[padNum][freeSlot];
}

void ForceFeedback::EndEffects(int padNum, float fadeTime)
{
	assert(padNum >= 0 && padNum < MAX_CONTROLLERS);
	for(int i = 0; i < MAX_EFFECTS_PER_CONTROLLER; i++)
	{
		if(m_Effects[padNum][i].IsActive())
		{
			m_Effects[padNum][i].EndEffect();
		}
	}
}

void ForceFeedback::Update(float frameTimeMS)
{
	if(m_initType == InputController::kIN_360PAD)
	{
		MsgGetPad lMsg;
		Player_obj->PostAMessage(&lMsg);
		GameController *pPad = lMsg.GetPad();
		Assert(pPad);
		m_type = pPad->IsAttached(0) ? InputController::kIN_360PAD : InputController::kIN_JOYSTICK;
	}

	if(m_type != InputController::kIN_360PAD)
	{
		return;
	}

	// Update
	for(int padNum = 0; padNum < MAX_CONTROLLERS; padNum++)
	{	
		for(int e = 0; e < MAX_EFFECTS_PER_CONTROLLER; e++)
		{
			g_Controllers[padNum].lastState = g_Controllers[padNum].state;
			g_Controllers[padNum].dwResult = XInputGetState( padNum, &g_Controllers[padNum].state );

			if( g_Controllers[padNum].dwResult != ERROR_SUCCESS )
			{
				continue;
			}

			if(m_Effects[padNum][e].IsActive())
			{
				m_Effects[padNum][e].Update(frameTimeMS);
			}
		}


		int recordedRumbles[MAX_MOTORS];
		for(int m = 0; m < MAX_MOTORS; m++)
		{
			recordedRumbles[m] = 0;
			for(int e = 0; e < MAX_EFFECTS_PER_CONTROLLER; e++)
			{
				if(m_Effects[padNum][e].IsActive() && m_Effects[padNum][e].GetMotor() == m)
				{
					const int rumble = m_Effects[padNum][e].GetRumble();
					if(recordedRumbles[m] < rumble)
					{
						recordedRumbles[m] = rumble;
					}
				}
			}
		}

		XINPUT_VIBRATION amount;
		amount.wLeftMotorSpeed  = recordedRumbles[0];
		amount.wRightMotorSpeed = recordedRumbles[1];
		g_Controllers[padNum].dwResult = XInputSetState( padNum, &amount );

		/*
		if(!(recordedRumbles[0] == 0 || recordedRumbles[0] == MAX_RUMBLE))
		{
			int i = 0;
		}
		if( g_Controllers[padNum].dwResult != ERROR_SUCCESS )
		{
			assert(0);
		}*/
	}

}