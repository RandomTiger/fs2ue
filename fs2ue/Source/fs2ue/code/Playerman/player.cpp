

#ifdef FS2_UE
#include "Playerman/Player.h"
#include "Io/PadController.h"
#include "Behaviours/ScreenEffects.h"
#include "Cmdline/Cmdline.h"
#else
// Backwards compatibility for non unity build
#ifndef UNITY_BUILD
// Pretend we are a unity build
#define UNITY_BUILD
// Include ALL headers
#include "UnityBuild.h"
// Disable Pretend
#undef UNITY_BUILD
#endif
#endif

struct LocalPlayer
{
	int miObjNum;
	player *mpPlayer;
};

std::vector<ComponentBase *> gBehaviourAllocationList;
std::vector<LocalPlayer> PlayerList;
int PlayerLocalIndex = -1;

void AddPlayerBehaviours(object *pObject, const int lPlayerIndex);

int GetNumLocalPlayers()
{
	return PlayerList.size();
}

bool IsPlayerObj(const object * const pObj)
{
	if(Cmdline_splitscreen)
	{
		return (pObj->flags & OF_PLAYER_SHIP) != 0;
	}
	else
	{
		return pObj == Player_obj;
	}
}

bool AllPlayersAreDead(const bool state)
{
	if(Cmdline_splitscreen)
	{
		for(uint i = 0; i < PlayerList.size(); i++)
		{
			if((Ships[PlayerList[i].miObjNum].flags & SF_DYING) == 0)
			{
				return false;
			}
		}

		return true;
	}

	return state;
}

void ClearPlayerList()
{
	PlayerList.clear();
}


void SetPlayerShip(const int lPlayerShipNum)
{
	const int lObjNum = PlayerList[lPlayerShipNum].miObjNum;
	if(lObjNum == -1)
	{
		return;
	}

	Player_obj = &Objects[lObjNum];

	Player_ship		 = &Ships[lObjNum];
	Player_ai		 = &Ai_info[Player_ship->ai_index];
	Player			 = PlayerList[lPlayerShipNum].mpPlayer;
	PlayerLocalIndex = lPlayerShipNum;

	if (!Fred_running){
		Player->objnum = lObjNum;
	}
}

int GetLocalPlayerIndex()
{
	return PlayerLocalIndex;
}

void SetupPlayerShip(const int lObjNum, player *pPlayer)
{
	LocalPlayer lLocalPlayer;
	lLocalPlayer.miObjNum = lObjNum;
	lLocalPlayer.mpPlayer = pPlayer;

	// Extra setup required
	lLocalPlayer.mpPlayer->current_hotkey_set = -1;

	PlayerList.push_back(lLocalPlayer);
	const int lPlayerNum = PlayerList.size();
	const int lPlayerIndex = lPlayerNum - 1;

	SetPlayerShip(lPlayerIndex);

	Player_obj->flags |= OF_PLAYER_SHIP;			// make this object a player controlled ship.
	Player_obj->mComponentSysName = pPlayer->short_callsign;
	AddPlayerBehaviours(Player_obj, lPlayerIndex);
	//Player_obj->mBehaviours.AddBehaviour(g, kBehaviourController);

	Player_ai->targeted_subsys = NULL;
	Player_ai->targeted_subsys_parent = -1;

	Player->meGameState = GM_IN_MISSION;

	// determine if player start has initial velocity and set forward cruise percent to reflect this
	if ( Player_obj->phys_info.vel.z > 0.0f )
		Player->ci.forward_cruise_percent = Player_obj->phys_info.vel.z / Player_ship->current_max_speed * 100.0f;

	const bool lbDefaultToChaseMode = false;
	if(lbDefaultToChaseMode)
	{
		Viewer_mode |= VM_CHASE;
	}
}

void AddPlayerBehaviours(object *pObject, const int lPlayerIndex)
{
	pObject->AddBehaviour(PlayerList[lPlayerIndex].mpPlayer, kBehaviourPlayer);

	if(lPlayerIndex == 0)
	{
		Player_obj->AddBehaviour(GetPrimaryPad(), kBehaviourController);
	}
	else
	{
		Player_obj->AddBehaviour(GetSecondryPad(), kBehaviourController);
	}

	
	ScreenEffects *pScreenEffects = new ScreenEffects();
	pObject->AddBehaviour(pScreenEffects, kBehaviourScreenEffects);
	gBehaviourAllocationList.push_back(pScreenEffects);
}

void FlushAllAllocatedBeahviours()
{
	for(uint i = 0; i < gBehaviourAllocationList.size(); i++)
	{
		delete gBehaviourAllocationList[i];
		gBehaviourAllocationList[i] = 0;
	}

	gBehaviourAllocationList.clear();
}

bool player::IsDying()
{
	return (Ships[Objects[objnum].instance].flags & SF_DYING) == SF_DYING;
}

bool IsInDeadState(player *lpPlayer, const uint lState) 
{
	if(Cmdline_splitscreen)
	{
		return lpPlayer->IsDying();
	}
	else
	{
		return (Game_mode & lState) != 0;
	}
}

bool player::ProcessMessage(MessageBase *lpMsgBase)
{
	if(lpMsgBase->GetMessageID() == MsgFireStuff::GetStaticMessageID())
	{
		MsgFireStuff *lpMsg = (MsgFireStuff *) lpMsgBase;
		obj_player_fire_stuff( lpMsg->mpObj, ci );
		return true;
	}

	return false;
}