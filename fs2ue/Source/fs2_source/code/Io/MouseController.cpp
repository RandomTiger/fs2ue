#ifndef UNITY_BUILD
#include <windows.h>
#include <xinput.h>
#include "MouseController.h"
#include "InputController.h"
#include "PadController.h"
#include "Mouse.h"
#include "gamesequence.h"
#endif

MouseController g_MouseController;

// call once to init the mouse
void MouseController::Init(const int inputType)
{
	m_initType = inputType;
	m_type = inputType;
	m_xpos = m_ypos = 0;

	mouse_init();

	if(m_type == InputController::kIN_360PAD)
	{
		lastbuttonDownA = false;
		m_upCountA = 0;

		m_mousePad = 0;
		XINPUT_STATE state;
		DWORD dwResult = XInputGetState( m_mousePad, &state );
		if( dwResult != ERROR_SUCCESS )
		{
			m_type = InputController::kIN_MOUSE;
		}
	}
}

void MouseController::Update(const int frameTimeMS, const int minX, const int minY, const int maxX, const int maxY)
{
	if(m_initType == InputController::kIN_360PAD)
	{
		m_type = GetPrimaryPad()->IsAttached(0) ? InputController::kIN_360PAD : InputController::kIN_MOUSE;
	}

	if(m_type == InputController::kIN_360PAD)
	{
		// Deal with up count
		const bool buttonDownA = GetPrimaryPad()->IsButtonDown(m_mousePad, XINPUT_GAMEPAD_A);
		if(buttonDownA == false && lastbuttonDownA == true)
		{
			m_upCountA = 1;
		}
		lastbuttonDownA  = buttonDownA;

		int x, y;
		GetPrimaryPad()->GetPadAnalogStickPos(m_mousePad, x, y, true, 1);

		m_xpos += x * frameTimeMS / GameController::ANALOG_MAX_MOVE;
		m_ypos -= y * frameTimeMS / GameController::ANALOG_MAX_MOVE;

		m_xpos = min(m_xpos, maxX);
		m_xpos = max(m_xpos, minX);

		m_ypos = min(m_ypos, maxY);
		m_ypos = max(m_ypos, minY);
	}
}

void MouseController::MarkButton( uint flags, int set )
{
	mouse_mark_button( flags, set );
}

// Fills in xpos & ypos if not NULL.
// Returns Button states
// Always returns coordinates clipped to screen coordinates.
int MouseController::GetPos( int *xpos, int *ypos )
{
	if(m_type == InputController::kIN_360PAD)
	{
		int flags = 0;
		if(xpos)
		{ 
			*xpos = m_xpos;
		}

		if(ypos)
		{
			*ypos = m_ypos;
		}

		if(GetPrimaryPad()->IsButtonDown(m_mousePad, XINPUT_GAMEPAD_A))
		{
			flags |= MOUSE_LEFT_BUTTON;
		}
		if(GetPrimaryPad()->IsButtonDown(m_mousePad, XINPUT_GAMEPAD_Y))
		{
			flags |= MOUSE_MIDDLE_BUTTON;
		}
		if(GetPrimaryPad()->IsButtonDown(m_mousePad, XINPUT_GAMEPAD_X))
		{
			flags |= MOUSE_RIGHT_BUTTON;
		}

		return flags;
	}
	else if(m_type == InputController::kIN_MOUSE)
	{
		return mouse_get_pos( xpos, ypos );
	}
	
	return 0;
}

// get_real_pos could be negative.
void MouseController::GetRealPos(int *mx, int *my)
{
	mouse_get_real_pos(mx, my);
}

void MouseController::SetPos(int xpos,int ypos)
{
	if(m_type == InputController::kIN_360PAD)
	{
		m_xpos = xpos;
		m_ypos = ypos;
	}
	else
	{
		mouse_set_pos(xpos, ypos);
	}
}

// Returns the number of times button n went from up to down since last call
int MouseController::DownCount(int n, int reset_count)
{
	if(m_type == InputController::kIN_360PAD && gameseq_get_state() != GS_STATE_GAME_PLAY)
	{
		if(n == MouseController::MOUSE_LEFT_BUTTON && GetPrimaryPad()->IsButtonDown(m_mousePad, XINPUT_GAMEPAD_A))
		{
			return 1;
		}
		if(n == MouseController::MOUSE_MIDDLE_BUTTON && GetPrimaryPad()->IsButtonDown(m_mousePad, XINPUT_GAMEPAD_Y))
		{
			return 1;
		}
		if(n == MouseController::MOUSE_RIGHT_BUTTON && GetPrimaryPad()->IsButtonDown(m_mousePad, XINPUT_GAMEPAD_X))
		{
			return 1;
		}
	}
	else
	{
		return mouse_down_count(n, reset_count);
	}

	return 0;
}
// Returns the number of times button n went from down to up since last call
int MouseController::UpCount(int n)
{
	int result = 0;
	if(m_type == InputController::kIN_360PAD)
	{
		if(n == MouseController::MOUSE_LEFT_BUTTON)
		{
			result = m_upCountA;
			m_upCountA = 0;
		}
	}
	else
	{
		result = mouse_up_count(n);
	}

	return result;
}

void MouseController::Flush()
{
	mouse_flush();
}

// returns 1 if mouse button btn is down, 0 otherwise
int MouseController::IsDown(int btn)
{
	if(m_type == InputController::kIN_360PAD && gameseq_get_state() != GS_STATE_GAME_PLAY)
	{
		if(btn == MouseController::MOUSE_LEFT_BUTTON && GetPrimaryPad()->IsButtonDown(m_mousePad, XINPUT_GAMEPAD_A))
		{
			return 1;
		}
		if(btn == MouseController::MOUSE_MIDDLE_BUTTON && GetPrimaryPad()->IsButtonDown(m_mousePad, XINPUT_GAMEPAD_Y))
		{
			return 1;
		}
		if(btn == MouseController::MOUSE_RIGHT_BUTTON && GetPrimaryPad()->IsButtonDown(m_mousePad, XINPUT_GAMEPAD_X))
		{
			return 1;
		}
	}
	else
	{
		return mouse_down(btn);
	}

	return 0;
}

// returns the fraction of time btn has been down since last call
// Not in use
float MouseController::DownTime(int btn)
{
	return mouse_down_time(btn);
}

// returns 1 if mouse is visible, 0 otherwise
int MouseController::IsVisible()
{
	return m_mouseStates.m_mouseHiddenCount == 0;	
}

void MouseController::EvalDeltas()
{
	mouse_eval_deltas();
}
void MouseController::GetDelta(int *dx, int *dy, int *dz, int *dw, int index)
{
	if(m_type == InputController::kIN_360PAD)
	{
		GetPrimaryPad()->GetPadAnalogStickPos(m_mousePad, *dx, *dy, index == 0);
		if(dx)
		{
			*dx /= 10000;
		}
		if(dy)
		{
			*dy /= 10000;
		}
	}
	else
	{
		mouse_get_delta(dx, dy, dz);
	}
}






