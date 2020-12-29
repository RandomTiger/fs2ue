#ifndef __MOUSECONTROLLER_H__
#define __MOUSECONTROLLER_H__

#ifndef UNITY_BUILD
#include "GlobalIncs/PsTypes.h"
#include "ControlsConfig.h"
#endif

class MouseController
{
private:

	struct MouseStates
	{
		MouseStates() : m_state(0), m_mouseSensitivity(4), m_mouseHiddenCount(0) {}

		int m_state;
		int m_mouseSensitivity;
		int m_mouseHiddenCount;
	};

	MouseStates m_mouseStates;
	int m_type;
	int m_initType;
	int m_xpos, m_ypos;
	int m_mousePad;

	bool lastbuttonDownA;
	int  m_upCountA;

public:

	enum
	{
		MOUSE_LEFT_BUTTON	= (1<<0),
		MOUSE_RIGHT_BUTTON	= (1<<1),
		MOUSE_MIDDLE_BUTTON	= (1<<2),

		MOUSE_NUM_BUTTONS	= 3,

		// keep the following two #defines up to date with the #defines above
		MOUSE_LOWEST_BUTTON		= (1<<0),
		MOUSE_HIGHEST_BUTTON	= (1<<2),
	};

	enum
	{
		MOUSE_STATE_USE_TO_FLY    = (1<<0),
		MOUSE_STATE_KEEP_CENTERED = (1<<1),
	};

	// call once to init the mouse
	void Init(const int inputType);
	void Update(const int frameTimeMS, const int minX, const int minY, const int maxX, const int maxY);
	void UpdateInputControls();

	void SetType(const int type) {m_type = type;}
	int GetType() {return m_type;}

	void MarkButton( uint flags, int set );

	// Fills in xpos & ypos if not NULL.
	// Returns Button states
	// Always returns coordinates clipped to screen coordinates.
	int GetPos( int *xpos, int *ypos );

	// get_real_pos could be negative.
	void GetRealPos(int *mx, int *my);

	void SetPos(int xpos,int ypos);



	// Returns the number of times button n went from up to down since last call
	int DownCount(int n, int reset_count = 1);
	// Returns the number of times button n went from down to up since last call
	int UpCount(int n);

	void Flush();

	int IsDown(int btn);			// returns 1 if mouse button btn is down, 0 otherwise
	float DownTime(int btn);	// returns the fraction of time btn has been down since last call
	int IsVisible();				// returns 1 if mouse is visible, 0 otherwise

	void EvalDeltas();
	void GetDelta(int *dx = NULL, int *dy = NULL, int *dz = NULL, int *dw = NULL, int index = 0);

	// MouseStates accessors
	void SetMouseState(const int flag)   { m_mouseStates.m_state |= flag;  }
	void UnsetMouseState(const int flag) { m_mouseStates.m_state &= ~flag; }

	void SetMouseSensitivity(const int value) { m_mouseStates.m_mouseSensitivity = value;}
	void SetMouseHidden(const int value)	  { m_mouseStates.m_mouseHiddenCount = value; }
	void IncMouseHidden()					  { m_mouseStates.m_mouseHiddenCount++; }
	void DecMouseHidden()					  { m_mouseStates.m_mouseHiddenCount--; }

	bool GetMouseState(const int flag) const {return (m_mouseStates.m_state & flag) == flag; }
	int  GetMouseSensitivity() const		 {return m_mouseStates.m_mouseSensitivity;}
	int  GetMouseHidden() const			     {return m_mouseStates.m_mouseHiddenCount;}
};

extern MouseController g_MouseController;

#endif
