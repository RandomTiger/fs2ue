#ifndef __INPUTCONTROLLER_H__
#define __INPUTCONTROLLER_H__

class InputController
{
	
public:

	enum
	{
	 	kIN_NONE     = 0,
		kIN_MOUSE    = 1 << 1,
		kIN_360PAD   = 1 << 2,
		kIN_JOYSTICK = 1 << 3,
		kIN_KEYBOARD = 1 << 4,
		kIN_DEFAULT  = 1 << 5,
	};

};

#endif
