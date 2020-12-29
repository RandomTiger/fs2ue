#if defined(FS2_UE)
#include "FREESPACE2/Horde.h"
#else
// Backwards compatibility for non unity build
#ifdef UNITY_BUILD
#include "UnityBuild.h"
#else
// Pretend we are a unity build
#define UNITY_BUILD
// Include ALL headers
#include "UnityBuild.h"
// Disable Pretend
#undef UNITY_BUILD
#endif
#endif

#if defined(HORDE_MODE)

bool BuyShip(const int index);
int SpawnShip(const char * const lNameOfShipPof, int lTeam);
void SetupPriceList();

struct EnemyItem
{
	EnemyItem(const char * const lName, const int lId, const int lPrice, const int lScore) : 
		mName(lName), mMoney(lPrice), mScore(lScore), mInUse(true), mId(lId)
	{}
	const char *mName;
	int mScore;
	int mMoney;
	bool mInUse;
	int mId;
};


class ExpandingList : public std::vector<EnemyItem>
{
public:
	 void Add(EnemyItem &lItem)
	 {
		for(unsigned int i = 0; i < size(); i++)
		{
			
			if(this->at(i).mInUse == false)
			{
				(*this)[i] = lItem;
				return;
			}
		}
		push_back(lItem);
	 }

};


struct ShopItem
{
	ShopItem(const char * const lName, int lPrice) : mName(lName), mPrice(lPrice) {}
	const char *mName;
	int mPrice;
};

struct sHordeInfo
{
	static float kStartMoney;

	int mWaveCount;
	int mMoney;
	int mScore;
	ExpandingList mEnemyList; // todo, break into groups?
	std::vector<int> mAllyList;  // todo, reenforcemences
};

float sHordeInfo::kStartMoney = 0;
sHordeInfo gHordeInfo;

enum
{
	kActionShipSelect,
	kActionInSession,
	kActionWin,
};

void AddEnemy(const char * const lName, const int lCash, const int lScore )
{
	const int lId = SpawnShip(lName, TEAM_HOSTILE);
	EnemyItem lEnemy(lName, lId, lCash, lScore);
	gHordeInfo.mEnemyList.Add(lEnemy);
}

class ActionShipSelect : public TomLib::StateMachine::Action 
{
public:
	virtual void Start() 
	{
		Player_ship->team = TEAM_FRIENDLY;
		gHordeInfo.mWaveCount++;
		mRequestEnd = false;
		mTimeout = 20.0f;
		BuyShip(1);
	}

	virtual bool Process(const float lTime) 
	{
		mTimeout -= lTime;

		return mRequestEnd || mTimeout <= 0;
	}

	virtual int End() 
	{
		int lCount = gHordeInfo.mWaveCount;

		switch(lCount)
		{
		case 1:
			AddEnemy("fighter2s-01.pof", 500, 500 );
			AddEnemy("fighter2s-01.pof", 500, 500 );
			AddEnemy("fighter2s-01.pof", 500, 500 );
			break;
		case 2:
			AddEnemy("fighter2s-02.pof", 500, 500 );
			AddEnemy("fighter2s-02.pof", 500, 500 );
			AddEnemy("fighter2s-02.pof", 500, 500 );

			AddEnemy("bomber10.pof", 500, 500 );
			AddEnemy("bomber10.pof", 500, 500 );
			AddEnemy("bomber10.pof", 500, 500 );
			break;
		case 3:
			AddEnemy("corvette2s-01.POF", 2500, 2500 );

			AddEnemy("fighter2s-03.pof", 500, 500 );
			AddEnemy("fighter2s-03.pof", 500, 500 );
			AddEnemy("fighter2s-03.pof", 500, 500 );

			AddEnemy("bomber10.pof", 500, 500 );
			AddEnemy("bomber10.pof", 500, 500 );
			AddEnemy("bomber10.pof", 500, 500 );

			AddEnemy("fighter2s-03.pof", 500, 500 );
			AddEnemy("fighter2s-03.pof", 500, 500 );
			AddEnemy("fighter2s-03.pof", 500, 500 );

			AddEnemy("fighter2s-03.pof", 500, 500 );
			AddEnemy("fighter2s-03.pof", 500, 500 );
			AddEnemy("fighter2s-03.pof", 500, 500 );
			break;
		case 4:
			AddEnemy("cruiser2s-01.POF", 10000, 10000 );


			AddEnemy("fighter2s-03.pof", 500, 500 );
			AddEnemy("fighter2s-03.pof", 500, 500 );
			AddEnemy("fighter2s-03.pof", 500, 500 );

			AddEnemy("fighter2s-03.pof", 500, 500 );
			AddEnemy("fighter2s-03.pof", 500, 500 );
			AddEnemy("fighter2s-03.pof", 500, 500 );

			AddEnemy("bomber10.pof", 500, 500 );
			AddEnemy("bomber10.pof", 500, 500 );
			AddEnemy("bomber10.pof", 500, 500 );
			break;
		case 8:
			AddEnemy("capital2s-01.POF", 1000, 1000 );

			AddEnemy("fighter2s-03.pof", 500, 500 );
			AddEnemy("fighter2s-03.pof", 500, 500 );
			AddEnemy("fighter2s-03.pof", 500, 500 );

			AddEnemy("bomber10.pof", 500, 500 );
			AddEnemy("bomber10.pof", 500, 500 );
			AddEnemy("bomber10.pof", 500, 500 );

			AddEnemy("fighter2s-03.pof", 500, 500 );
			AddEnemy("fighter2s-03.pof", 500, 500 );
			AddEnemy("fighter2s-03.pof", 500, 500 );

			AddEnemy("bomber10.pof", 500, 500 );
			AddEnemy("bomber10.pof", 500, 500 );
			AddEnemy("bomber10.pof", 500, 500 );
			break;
		case 10:

			AddEnemy("fighter2s-03.pof", 500, 500 );
			AddEnemy("fighter2s-03.pof", 500, 500 );
			AddEnemy("fighter2s-03.pof", 500, 500 );

			AddEnemy("bomber10.pof", 500, 500 );
			AddEnemy("bomber10.pof", 500, 500 );
			AddEnemy("bomber10.pof", 500, 500 );

			AddEnemy("fighter2s-03.pof", 500, 500 );
			AddEnemy("fighter2s-03.pof", 500, 500 );
			AddEnemy("fighter2s-03.pof", 500, 500 );

			AddEnemy("bomber10.pof", 500, 500 );
			AddEnemy("bomber10.pof", 500, 500 );
			AddEnemy("bomber10.pof", 500, 500 );
			break;
		}
		//	bomber2s-01.POF
		//	bomber2s-02.POF

		return kActionInSession;
	}
	float GetTimeout() {return mTimeout;}
	void RequestEnd() {mRequestEnd = true;}

private:
	bool mRequestEnd;
	float mTimeout;
};

class ActionInSession : public TomLib::StateMachine::Action
{
public:
	virtual void Start() {}
	virtual bool Process(const float lTime) 
	{
		bool lFoundAliveEnemy = false;

		for(unsigned int i = 0; i < gHordeInfo.mEnemyList.size(); i++)
		{
			if(gHordeInfo.mEnemyList[i].mInUse == false)
			{
				continue;
			}

			const int lObjNum = gHordeInfo.mEnemyList[i].mId;
			if(Objects[lObjNum].hull_strength > 0)
			{
				//if((Objects[lObjNum].flags & OF_SHOULD_BE_DEAD) == 0)
				lFoundAliveEnemy = true;
			}
			else
			{
				// Remove from list
				gHordeInfo.mScore += gHordeInfo.mEnemyList[i].mScore;
				gHordeInfo.mMoney += gHordeInfo.mEnemyList[i].mMoney;
				gHordeInfo.mEnemyList[i].mInUse = false;
			}
			
		}
		
		return lFoundAliveEnemy == false;
	}
	virtual int End() {return kActionWin;}
};

class ActionWin : public TomLib::StateMachine::Action
{
public:
	virtual void Start() {}
	virtual bool Process(const float lTime) {return true;}
	virtual int End() {return kActionShipSelect;}
};

ActionShipSelect gActionShipSelect;
ActionInSession gActionInSession;
ActionWin gActionWin;
TomLib::StateMachine gHordeSMachine(3);

void HordeInit()
{
	gHordeInfo.mWaveCount = 0;
	gHordeInfo.mMoney = sHordeInfo::kStartMoney;

	gHordeSMachine.Add(kActionShipSelect, &gActionShipSelect);
	gHordeSMachine.Add(kActionInSession, &gActionInSession);
	gHordeSMachine.Add(kActionWin, &gActionWin);
}

void HordeDeinit()
{

}

void HordeStart()
{
	gHordeInfo.mWaveCount = 0;
	SetupPriceList();

	gHordeSMachine.Start();
}

void HordeEnd()
{
}

std::vector<ShopItem> gPriceList;

void SetupPriceList()
{
	gPriceList.clear();
	gPriceList.push_back(ShopItem("Wingman",     200));
	gPriceList.push_back(ShopItem("FighterWing", 500));
	gPriceList.push_back(ShopItem("BomberWing",  500));
	gPriceList.push_back(ShopItem("Cruiser",    1000));
	gPriceList.push_back(ShopItem("Corvette",   2500));
	gPriceList.push_back(ShopItem("Capital",   10000));
}

bool BuyShip(const int index)
{
	const ShopItem &lItem = gPriceList[index];

	if((gHordeInfo.mMoney - lItem.mPrice) < 0)
	{
		return false;
	}

	gHordeInfo.mMoney -= lItem.mPrice;

	if(strcmp(lItem.mName, "Wingman") == 0)
	{
		SpawnShip("fighter01.pof", TEAM_FRIENDLY);
	}
	else if(strcmp(lItem.mName, "FighterWing") == 0)
	{
		SpawnShip("fighter01.pof", TEAM_FRIENDLY);
		SpawnShip("fighter01.pof", TEAM_FRIENDLY);
		SpawnShip("fighter01.pof", TEAM_FRIENDLY);
	}
	else if(strcmp(lItem.mName, "BomberWing") == 0)
	{
		SpawnShip("bomber2t-01.POF", TEAM_FRIENDLY);
		SpawnShip("bomber2t-01.POF", TEAM_FRIENDLY);
		SpawnShip("bomber2t-01.POF", TEAM_FRIENDLY);
		// bomber2t-03.POF
	}
	else if(strcmp(lItem.mName, "Corvette") == 0)
	{
		SpawnShip("corvette2t-01.POF", TEAM_FRIENDLY);
	}
	else if(strcmp(lItem.mName, "Capital") == 0)
	{
		SpawnShip("capital2t-01.POF", TEAM_FRIENDLY);
	}
	else if(strcmp(lItem.mName, "Cruiser") == 0)
	{
		SpawnShip("cruiser2t-01.POF", TEAM_FRIENDLY);
	}

	return true;
}

void HordeUpdate(float flFrametime)
{
	gHordeSMachine.Update(flFrametime);
}

void HordeUpdateInput(const int key)
{
	if(gHordeSMachine.GetCurrentState() == kActionShipSelect)
	{
		if(key >= KEY_1 && key <= KEY_9)
		{
			const unsigned int value = key - KEY_1;
			if(value < gPriceList.size())
			{
				BuyShip(value);
			}
		}

		if(key == KEY_ENTER)
		{
			gActionShipSelect.RequestEnd();
		}
	}

	
}


void HordeRender()
{
	const int lStrLen = 128;
	char lStr[lStrLen];

	int lWidth, lHalfWidth = gr_screen.clip_width/ 2;

	int lCurrentY = 140;
	int lSpacingY = 20;

	gr_set_color_fast(&Color_text_normal);
	gr_set_font(FONT2);

	sprintf_s(lStr, lStrLen, "HORDE!");
	gr_get_string_size(&lWidth, NULL, lStr);
	gr_string(lHalfWidth - lWidth / 2,lCurrentY, lStr);
	lCurrentY += lSpacingY;

	sprintf_s(lStr, lStrLen, "WAVE %d", gHordeInfo.mWaveCount);
	gr_get_string_size(&lWidth, NULL, lStr);
	gr_string(lHalfWidth - lWidth / 2, lCurrentY, lStr);
	lCurrentY += lSpacingY;

	sprintf_s(lStr, lStrLen, "CASH %d", gHordeInfo.mMoney);
	gr_get_string_size(&lWidth, NULL, lStr);
	gr_string(lHalfWidth - lWidth / 2, lCurrentY, lStr);
	lCurrentY += lSpacingY;

	sprintf_s(lStr, lStrLen, "SCORE %d", gHordeInfo.mScore);
	gr_get_string_size(&lWidth, NULL, lStr);
	gr_string(lHalfWidth - lWidth / 2, lCurrentY, lStr);
	lCurrentY += lSpacingY;

	gr_set_font(FONT2);
	if(gHordeSMachine.GetCurrentState() == kActionShipSelect)
	{
		lSpacingY = 15;
		lCurrentY += lSpacingY;

		for(unsigned int i = 0; i < gPriceList.size(); i++)
		{
			if(gPriceList[i].mPrice <= gHordeInfo.mMoney)
			{
				gr_set_color_fast(&Color_text_normal);
			}
			else
			{
				gr_set_color_fast(&Color_red);
			}

			sprintf_s(lStr, lStrLen, "%d %s %d", i + 1, gPriceList[i].mName, gPriceList[i].mPrice);
			gr_get_string_size(&lWidth, NULL, lStr);
			gr_string(lHalfWidth - lWidth / 2, lCurrentY, lStr);
			lCurrentY += lSpacingY;

		}

		lCurrentY += lSpacingY;
		gr_set_color_fast(&Color_text_normal);

		sprintf_s(lStr, lStrLen, "Press ENTER to start wave (autostart in %.f)", gActionShipSelect.GetTimeout());
		gr_get_string_size(&lWidth, NULL, lStr);
		gr_string(lHalfWidth - lWidth / 2, lCurrentY, lStr);
	}
}

int SpawnShip(const char * const lNameOfShipPof, int lTeam)
{
	int ShipNumType = -1;
	for(int idx=0; idx<Num_ship_types; idx++)
	{
		const char * const lNameOfShipPofToCheck = Ship_info[idx].pof_file;
		printf("%s", lNameOfShipPofToCheck);
		if(!stricmp(lNameOfShipPofToCheck, lNameOfShipPof))
		{
			ShipNumType = idx;
			break;
		}
	}

	if(ShipNumType < 0)
	{
		Assert(0);
		return -1;
	}

   Assert((Game_mode & GM_IN_MISSION) && (Player_obj != NULL));
	
	vector add = Player_obj->pos;
	add.x += frand_range(-700.0f, 700.0f);
	add.y += frand_range(-700.0f, 700.0f);
	add.z += frand_range(-700.0f, 700.0f);

	int objnum = ship_create(&vmd_identity_matrix, &add, ShipNumType);
	if(objnum < 0)
	{
		Assert(0);
		return -1;
	}

	int collided = 0;
	object *hit_check;
	ship *s_check;
	object *new_obj = &Objects[objnum];

	// place him
	// now make sure we're not colliding with anyone		
	do {
		collided = 0;
		ship_obj *moveup = GET_FIRST(&Ship_obj_list);
		while(moveup!=END_OF_LIST(&Ship_obj_list))
		{

			// don't check the new_obj itself!!
			if(moveup->objnum != objnum)
			{
				
				hit_check = &Objects[moveup->objnum];
				
				Assert(hit_check->type == OBJ_SHIP);
				Assert(hit_check->instance >= 0);

				if((hit_check->type != OBJ_SHIP) || (hit_check->instance < 0)){
					continue;
				}
				s_check = &Ships[hit_check->instance];
				
				// just to make sure we don't get any strange magnitude errors
				if(vm_vec_same(&hit_check->pos, &Objects[objnum].pos)){
					Objects[objnum].pos.x += 1.0f;
				}
				
				WITHIN_BBOX();				
				if(collided){						
					MOVE_AWAY_BBOX();
					break;
				} 
				collided = 0;
			}
			moveup = GET_NEXT(moveup);
		}
	} while(collided);   					

	int lInstanceNum = Objects[objnum].instance;
	Assert(lInstanceNum >= 0);
	Ships[lInstanceNum].team = lTeam;
	
	// warpin
	shipfx_warpin_start(&Objects[objnum]);

	// tell him to attack				
	ai_add_ship_goal_player( AIG_TYPE_PLAYER_SHIP, AI_GOAL_CHASE_ANY, SM_ATTACK, NULL, &Ai_info[Ships[lInstanceNum].ai_index] );

	return objnum;
}

/*

bomber04.pof
bomber05.pof
bomber06.pof
bomber08.pof
bomber09.POF
bomber10.POF

bomber2v-02.POF

capital01.POF
capital01a.POF
capital01b.POF
capital02.POF
capital03.POF
capital04.POF
capital05.POF


capital2v-01.POF
corvette2v-01.POF

corvette2r-01.POF

cruiser01.POF
cruiser02.POF
cruiser03.POF

cruiser2v-01.POF

fighter01.POF
fighter03.POF
fighter06.POF
fighter07.POF
fighter08.POF
fighter09.POF
fighter10.POF
fighter11.POF
fighter13.POF



fighter2t-01.POF
fighter2t-02.POF
fighter2t-03.POF
fighter2t-04.POF
fighter2t-05.POF
fighter2v-01.POF
fighter2v-02.POF
fighter2v-03.POF
fighter2v-04.POF

supercap2s-01.POF
TerranSuper.POF
*/
#endif