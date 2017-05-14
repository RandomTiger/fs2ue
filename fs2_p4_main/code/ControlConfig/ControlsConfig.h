/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#ifndef CONTROLS_CONFIG_H
#define CONTROLS_CONFIG_H

#define CONTROL_CONFIG_XSTR	507

#define JOY_X_AXIS	0
#define JOY_Y_AXIS	1
#define JOY_Z_AXIS	2
#define JOY_RX_AXIS	3
#define JOY_RY_AXIS	4
#define JOY_RZ_AXIS	5

enum
{
	JOY_YAW_AXIS,
	JOY_PITCH_AXIS,
	JOY_ROLL_AXIS,
	JOY_ABS_THROTTLE_AXIS,
	JOY_REL_THROTTLE_AXIS,
	NUM_JOY_AXIS_ACTIONS,
};

// --------------------------------------------------
// different types of controls that can be assigned 
// --------------------------------------------------

#define CC_TYPE_TRIGGER		0
#define CC_TYPE_CONTINUOUS	1

typedef struct config_item {
	short key_default;  // default key bound to action
	short joy_default;  // default joystick button bound to action
	char tab;				// what tab (catagory) it belongs in
	char *text;				// describes the action in the config screen
	char type;				// manner control should be checked in
	short key_id;  // actual key bound to action
	short joy_id;  // joystick button bound to action
	int used;				// has control been used yet in mission?  If so, this is the timestamp
} config_item;

// --------------------------------------------------
// Keyboard #defines for the actions  
// This is the value of the id field in config_item
// --------------------------------------------------

enum INPUT_ACTIONS
{
// targeting a ship
	TARGET_NEXT,
	TARGET_PREV,
	TARGET_NEXT_CLOSEST_HOSTILE,
	TARGET_PREV_CLOSEST_HOSTILE,
	TOGGLE_AUTO_TARGETING,
	TARGET_NEXT_CLOSEST_FRIENDLY,
	TARGET_PREV_CLOSEST_FRIENDLY,
	TARGET_SHIP_IN_RETICLE,
	TARGET_CLOSEST_SHIP_ATTACKING_TARGET,
	TARGET_LAST_TRANMISSION_SENDER,
	STOP_TARGETING_SHIP,

// targeting a ship's subsystem
	TARGET_SUBOBJECT_IN_RETICLE,
	TARGET_NEXT_SUBOBJECT,
	TARGET_PREV_SUBOBJECT,
	STOP_TARGETING_SUBSYSTEM,

// speed matching 
	MATCH_TARGET_SPEED,
	TOGGLE_AUTO_MATCH_TARGET_SPEED,

// weapons
	FIRE_PRIMARY,
	FIRE_SECONDARY,
	CYCLE_NEXT_PRIMARY,
	CYCLE_PREV_PRIMARY,
	CYCLE_SECONDARY,
	CYCLE_NUM_MISSLES,
	LAUNCH_COUNTERMEASURE,

// controls
	FORWARD_THRUST,
	REVERSE_THRUST,
	BANK_LEFT,
	BANK_RIGHT,
	PITCH_FORWARD,
	PITCH_BACK,
	YAW_LEFT,
	YAW_RIGHT,

// throttle control
	ZERO_THROTTLE,
	MAX_THROTTLE,
	ONE_THIRD_THROTTLE,
	TWO_THIRDS_THROTTLE,
	PLUS_5_PERCENT_THROTTLE,
	MINUS_5_PERCENT_THROTTLE,

// squadmate messaging keys
	ATTACK_MESSAGE,
	DISARM_MESSAGE,
	DISABLE_MESSAGE,
	ATTACK_SUBSYSTEM_MESSAGE,
	CAPTURE_MESSAGE,
	ENGAGE_MESSAGE,
	FORM_MESSAGE,
	IGNORE_MESSAGE,
	PROTECT_MESSAGE,
	COVER_MESSAGE,
	WARP_MESSAGE,
	REARM_MESSAGE,

	TARGET_CLOSEST_SHIP_ATTACKING_SELF,

// Views
	VIEW_CHASE,
	VIEW_EXTERNAL,
	VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK,
	VIEW_SLEW,
	VIEW_OTHER_SHIP,
	VIEW_DIST_INCREASE,
	VIEW_DIST_DECREASE,
	VIEW_CENTER,
	PADLOCK_UP,
	PADLOCK_DOWN,
	PADLOCK_LEFT,
	PADLOCK_RIGHT,


	RADAR_RANGE_CYCLE,
	SQUADMSG_MENU,
	SHOW_GOALS,
	END_MISSION,
	TARGET_TARGETS_TARGET,
	AFTERBURNER,

	INCREASE_WEAPON,
	DECREASE_WEAPON,
	INCREASE_SHIELD,
	DECREASE_SHIELD,
	INCREASE_ENGINE,
	DECREASE_ENGINE,
	ETS_EQUALIZE,
	SHIELD_EQUALIZE,
	SHIELD_XFER_TOP,
	SHIELD_XFER_BOTTOM,
	SHIELD_XFER_LEFT,
	SHIELD_XFER_RIGHT,

	XFER_SHIELD,
	XFER_LASER,
	SHOW_DAMAGE_POPUP, // AL: this binding should be removing next time the controls are reorganized

	BANK_WHEN_PRESSED,
	SHOW_NAVMAP,
	ADD_REMOVE_ESCORT,
	ESCORT_CLEAR,
	TARGET_NEXT_ESCORT_SHIP,

	TARGET_CLOSEST_REPAIR_SHIP,
	TARGET_NEXT_UNINSPECTED_CARGO,
	TARGET_PREV_UNINSPECTED_CARGO,
	TARGET_NEWEST_SHIP,
	TARGET_NEXT_LIVE_TURRET,
	TARGET_PREV_LIVE_TURRET,

	TARGET_NEXT_BOMB,
	TARGET_PREV_BOMB,

// multiplayer messaging keys
	MULTI_MESSAGE_ALL,
	MULTI_MESSAGE_FRIENDLY,
	MULTI_MESSAGE_HOSTILE,
	MULTI_MESSAGE_TARGET,

// multiplayer misc keys
	MULTI_OBSERVER_ZOOM_TO,

	TIME_SPEED_UP,
	TIME_SLOW_DOWN,

	TOGGLE_HUD_CONTRAST,

	MULTI_TOGGLE_NETINFO,

	MULTI_SELF_DESTRUCT,

	CHOOSE_SINGLE_SECONDARY,
	CHOOSE_DUAL_SECONDARY,

	CHOOSE_PRIMARY_1_OFF,
	CHOOSE_PRIMARY_2_OFF,
	CHOOSE_PRIMARY_ALL_ON,
	CCFG_MAX
};
// this should be the total number of control action defines above (or last define + 1)
//#define CCFG_MAX 107

extern int Failed_key_index;
extern int Invert_heading;
extern int Invert_pitch;
extern int Invert_roll;
extern int Invert_thrust;
extern int Disable_axis2;
extern int Disable_axis3;

struct AxisMap
{
	static int *GetAxisMapTo(bool forceNonPad = false);
	static int *GetAxisMapToDefaults(bool forceNonPad = false);

	static int Axis_map_to[];
	static int Axis_map_to_360_pad[];
	static int Axis_map_to_defaults[];
	static int Axis_map_to_defaults_360_pad[];
};

extern int Invert_axis[];
extern int Invert_axis_defaults[];

extern config_item Control_config[];	// stores the keyboard configuration
extern char **Scan_code_text;
extern char **Joy_button_text;

// initialize common control config stuff - call at game startup after localization has been initialized
void control_config_common_init();

void control_config_init();
void control_config_do_frame(float frametime);
void control_config_close();

void control_config_cancel_exit();

void control_config_reset_defaults();
int translate_key_to_index(char *key);
char *translate_key(char *key);
char *textify_scancode(int code);
float check_control_timef(int id);
int check_control(int id, int key = -1);
void control_get_axes_readings(int *h, int *p, int *b, int *ta, int *tr);
void control_used(int id);
void control_config_clear();
void clear_key_binding(short key);
void control_check_indicate();
void control_config_clear_used_status();

#endif
