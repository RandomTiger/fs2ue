/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _OBJECT_H
#define _OBJECT_H

#ifndef UNITY_BUILD
#include "TomLib/src/ComponentSystem/ComponentSystem.h"
#include "vecmat.h"
#include "physics.h"
#include "cfile.h"
#endif

class GameController;
struct player;
struct object;

enum
{
	kBehaviourPlayer,
	kBehaviourController,
	kBehaviourScreenEffects,
};

class MsgFireStuff : public MessageBase
{
	MESSAGE_CLASS()
public:
	MsgFireStuff(object * const lpObject) : mpObj(lpObject) {}
private:
	object *mpObj;
	friend struct player;
};

/*
 *		CONSTANTS
 */

#ifdef FS2_DEMO
	#define MAX_OBJECTS			300		
#else
	#define MAX_OBJECTS			1000		
#endif

#define MAX_SHIELD_SECTIONS	4					//	Number of sections in shield.

#ifndef NDEBUG
#define OBJECT_CHECK 
#endif

//	Team bitmasks.
#define TEAM_HOSTILE		(1 << 0)
#define TEAM_FRIENDLY	(1 << 1)
#define TEAM_NEUTRAL		(1 << 2)
#define TEAM_UNKNOWN		(1 << 3)
#define TEAM_TRAITOR		(1	<< 4)
#define TEAM_ANY			(TEAM_HOSTILE|TEAM_FRIENDLY|TEAM_NEUTRAL|TEAM_UNKNOWN|TEAM_TRAITOR)
#define MAX_TEAM_NAMES_INDEX	TEAM_TRAITOR

//Object types
#define OBJ_NONE				0		//unused object
#define OBJ_SHIP				1		//a ship
#define OBJ_WEAPON			2		//a laser, missile, etc
#define OBJ_FIREBALL			3		//an explosion
#define OBJ_START				4		//a starting point marker (player start, etc)
#define OBJ_WAYPOINT			5		//a waypoint object, maybe only ever used by Fred
#define OBJ_DEBRIS			6		//a flying piece of ship debris
#define OBJ_CMEASURE			7		//a countermeasure, such as chaff
#define OBJ_GHOST				8		//so far, just a placeholder for when a player dies.
#define OBJ_POINT				9		//generic object type to display a point in Fred.
#define OBJ_SHOCKWAVE		10		// a shockwave
#define OBJ_WING				11		// not really a type used anywhere, but I need it for Fred.
#define OBJ_OBSERVER       12    // used for multiplayer observers (possibly single player later)
#define OBJ_ASTEROID			13		//	An asteroid, you know, a big rock, like debris, sort of.
#define OBJ_JUMP_NODE		14		// A jump node object, used only in Fred.
#define OBJ_BEAM				15		// beam weapons. we have to roll them into the object system to get the benefits of the collision pairs

//Make sure to change Object_type_names in Object.c when adding another type!
#define MAX_OBJECT_TYPES	16

#define UNUSED_OBJNUM		(-MAX_OBJECTS*2)	//	Newer systems use this instead of -1 for invalid object.

#ifndef NDEBUG
extern char	*Object_type_names[MAX_OBJECT_TYPES];
#endif

// each object type should have these functions:  (I will use weapon as example)
//
// int weapon_create( weapon specific parameters )
// {
//    ...
//		objnum = obj_create();
//		... Do some check to correctly handle obj_create returning  which
//        means that that object couldn't be created
//    ... Initialize the weapon-specific info in Objects[objnum]
//    return objnum;
// }
//
// void weapon_delete( object * obj )
// {
//    {Put a call to this in OBJECT.C, function obj_delete_all_that_should_be_dead }
//    WARNING: To kill an object, set it's OF_SHOULD_BE_DEAD flag.  Then,
//    this function will get called when it's time to clean up the data.
//    Assert( obj->flags & OF_SHOULD_BE_DEAD );
//    ...
//    ... Free up all weapon-specfic data
//    obj_delete(objnum);
// }
// 
// void weapon_move( object * obj )
// {
//    {Put a call to this in ??? }
//    ... Do whatever needs to be done each frame.  Usually this amounts
//        to setting the thrust, seeing if we've died, etc.
// }
//
// int weapon_check_collision( object * obj, object * other_obj, vector * hitpos )
// {
//    this should check if a vector from 
//		other_obj->last_pos to other_obj->pos with a radius of other_obj->radius
//    collides with object obj.   If it does, then fill in hitpos with the point
//    of impact and return non-zero, otherwise return 0 if no impact.   Note that
//    this shouldn't take any action... that happens in weapon_hit.
// }

// 
// void weapon_hit( object * obj, object * other_obj, vector * hitpos )
// {
//    {Put a call to this in COLLIDE.C}
//    ... Do what needs to be done when this object gets hit
//    ... Reducing shields, etc
// }

//Misc object flags
#define OF_RENDERS			(1<<0)	//It renders as something ( objtype_render gets called)
#define OF_COLLIDES			(1<<1)	//It collides with stuff (objtype_check_impact & objtype_hit gets called)
#define OF_PHYSICS			(1<<2)	//It moves with standard physics.
#define OF_SHOULD_BE_DEAD	(1<<3)	//this object should be dead, so next time we can, we should delete this object.
#define OF_INVULNERABLE		(1<<4)	//	invulnerable
#define OF_PROTECTED			(1<<5)	// Don't kill this object, probably mission-critical.
#define OF_PLAYER_SHIP		(1<<6)	// this object under control of some player -- don't do ai stuff on it!!!
#define OF_NO_SHIELDS		(1<<7)	// object has no shield generator system (i.e. no shileds)
#define OF_JUST_UPDATED		(1<<8)	// for multiplayer -- indicates that we received object update this frame
#define OF_COULD_BE_PLAYER (1<<9)	// for multiplayer -- indicates that it is selectable ingame joiners as their ship
#define OF_WAS_RENDERED		(1<<10)	//	Set if this object was rendered this frame.  Only gets set if OF_RENDERS set.  Gets cleared or set in obj_render_all().
#define OF_NOT_IN_COLL		(1<<11)	// object has not been added to collision list
#define OF_BEAM_PROTECTED	(1<<12)	// don't fire beam weapons at this type of object, probably mission critical.
#define OF_GUARDIAN			(1<<13)	// Don't allow ship to die, keep at least 1% hull
#define OF_SPECIAL_WARP		(1<<14)	// Object has special warp-in enabled.

// Flags used by Fred
#define OF_MARKED          (1<<16)   //Object is marked (Fred).  Can be reused in Freespace for anything that won't be used by Fred.
#define OF_TEMP_MARKED		(1<<17)	//Temporarily marked (Fred).
#define OF_REFERENCED		(1<<18)	// (Fred) Object is referenced by something somewhere
#define OF_HIDDEN				(1<<19)  // Object is hidden (not shown) and can't be manipulated

// max # of object sounds per object
#define MAX_OBJECT_SOUNDS	4

typedef struct object : public ComponentSystem {
	struct object	*next, *prev;	// for linked lists of objects
	int				signature;		// Every object ever has a unique signature...
	char				type;				// what type of object this is... robot, weapon, hostage, powerup, fireball
	int				parent;			// This object's parent.
	int				parent_sig;		// This object's parent's signature
	char				parent_type;	// This object's parent's type
	int				instance;		// which instance.  ie.. if type is Robot, then this indexes into the Robots array
	uint				flags;			// misc flags.  Call obj_set_flags to change this.
	vector			pos;				// absolute x,y,z coordinate of center of object
	matrix			orient;			// orientation of object in world
	float				radius;			// 3d size of object - for collision detection
	vector			last_pos;		// where object was last frame
	matrix			last_orient;	// how the object was oriented last frame
	physics_info	phys_info;		// a physics object
	float				shields[MAX_SHIELD_SECTIONS];	//	Shield is broken into components.  Quadrants on 4/24/97.
	float				hull_strength;	//	Remaining hull strength.
	short				objsnd_num[MAX_OBJECT_SOUNDS];		// Index of persistant sound struct.  -1 if no persistant sound assigned.
	ushort			net_signature;
	int				num_pairs;		// How many object pairs this is associated with.  When 0 then there are no more.
	int				m_renderFlags;

} object;

// object backup struct used by Fred.
typedef struct object_orient_pos {
	vector pos;
	matrix orient;
} object_orient_pos;

/*
 *		VARIABLES
 */

extern int Object_inited;
extern int Show_waypoints;

// The next signature for the next newly created object. Zero is bogus
#define OBJECT_SIG_SHIP_START					300000;				// ships start at this signature
extern int Object_next_ship_signature;
extern int Object_next_signature;		
extern int num_objects;

extern object Objects[];
extern int Highest_object_index;		//highest objnum
extern int Highest_ever_object_index;
extern object obj_free_list;
extern object obj_used_list;
extern object obj_create_list;

extern int render_total;
extern int render_order[MAX_OBJECTS];

extern object *Viewer_obj;	// Which object is the viewer. Can be NULL.
extern object *Player_obj;	// Which object is the player. Has to be valid.

// Use this instead of "objp - Objects" to get an object number
// given it's pointer.  This way, we can replace it will a macro
// to check that the pointer is valid for debugging.
#define OBJ_INDEX(objp) (objp-Objects)

/*
 *		FUNCTIONS
 */

//do whatever setup needs to be done
void obj_init();

//initialize a new object.  adds to the list for the given segment.
//returns the object number.  The object will be a non-rendering, non-physics
//object.  Returns 0 if failed, otherwise object index.
//You can pass 0 for parent if you don't care about that.
//You can pass null for orient and/or pos if you don't care.
int obj_create(ubyte type,int parent_obj, int instance, matrix * orient, vector * pos, float radius, uint flags );

//Render an object.  Calls one of several routines based on type
void obj_render(object *obj);

//Sorts and renders all the ojbects
enum
{
	OBJ_RENDER_NOT_SET,
	OBJ_RENDER_OPAQUE,
	OBJ_RENDER_TRANSPARENT,
};

void obj_render_all(void (*render_function)(object *objp), int renderFlags );

//move all objects for the current frame
void obj_move_all(float frametime);		// moves all objects

//move an object for the current frame
void obj_move_one(object * obj, float frametime);

// function to delete an object -- should probably only be called directly from editor code
void obj_delete(int objnum);

// should only be used by the editor!
void obj_merge_created_list(void);

// recalculate object pairs for an object
#define OBJ_RECALC_PAIRS(obj_to_reset)		do {	obj_set_flags(obj_to_reset, obj_to_reset->flags & ~(OF_COLLIDES)); obj_set_flags(obj_to_reset, obj_to_reset->flags | OF_COLLIDES); } while(0);

// Removes any occurances of object 'a' from the pairs list.
void obj_remove_pairs( object * a );

// add an object to the pairs list
void obj_add_pairs(int objnum);

//	Returns true if objects A and B are expected to collide in next duration seconds.
//	For purposes of this check, the first object moves from current location to predicted
//	location.  The second object is assumed to be where it will be at time duration, NOT
//	where it currently is.
//	radius_scale: 0.0f means use polygon models, else scale sphere size by radius_scale
//	radius_scale == 1.0f means Descent style collisions.
int objects_will_collide(object *A, object *B, float duration, float radius_scale);

// Used for pausing.  Seems hacked.  Was in PHYSICS, but that broke the TestCode program,
// so I moved it into the object lib.  -John
void obj_init_all_ships_physics();

float get_shield_strength(object *objp);
void set_shield_strength(object *objp, float strength);
void add_shield_strength(object *objp, float delta);

// returns the average 3-space position of all ships.  useful to find "center" of battle (sort of)
void obj_get_average_ship_pos(vector *pos);

// function to deal with firing player things like lasers, missiles, etc.
// separated out because of multiplayer issues.
void obj_player_fire_stuff( object *objp, const control_info &ci );

// Call this if you want to change an object flag so that the
// object code knows what's going on.  For instance if you turn
// off OF_COLLIDES, the object code needs to know this in order to
// actually turn the object collision detection off.  By calling
// this you shouldn't get Int3's in the checkobject code.  If you
// do, then put code in here to correctly handle the case.
void obj_set_flags(object *obj, uint new_flags);

// get the Ship_info flags for a given ship (if you have the object)
int obj_get_SIF(object *objp);
int obj_get_SIF(int obj);

// get the team for any object
int obj_team(object *objp);

void obj_move_all_pre(object *objp, float frametime);
void obj_move_all_post(object *objp, float frametime);

void obj_move_call_physics(object *objp, float frametime);

// multiplayer object update stuff begins -------------------------------------------

// do client-side pre-interpolation object movement
void obj_client_pre_interpolate();

// do client-side post-interpolation object movement
void obj_client_post_interpolate();

// move an observer object in multiplayer
void obj_observer_move(float flFrametime);


#endif
