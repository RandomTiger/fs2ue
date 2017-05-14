/*
 * Code created by Thomas Whittaker (Random Tiger) for a Freespace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

// To compile this source, /Zc:wchar_t- must be set

#ifndef UNITY_BUILD
// Need to disable the PSTypes.h define
#ifdef Assert
#undef Assert
#endif
#endif

#ifndef UNITY_BUILD
#include <sphelper.h>                           // Contains definitions of SAPI functions

#include "OsApi/OsApi.h"
#include "VoiceRecognition.h"
#include "grammar.h"

// Freespace includes
#include "hudsquadmsg.h"
#include "keycontrol.h"
#include "player.h"
#include "ship.h"
#endif

VoiceRecognition g_VoiceRecognition;

CComPtr<ISpRecoGrammar>         p_grammarObject; // Pointer to our grammar object
CComPtr<ISpRecoContext>         p_recogContext;  // Pointer to our recognition context
CComPtr<ISpRecognizer>			p_recogEngine;   // Pointer to our recognition engine instance



// Requires CoInitialize
bool VoiceRecognition::Init(int event_id, int grammar_id, int command_resource)
{
	HWND hWnd = (HWND) os_get_window();

    HRESULT hr = S_OK;
    CComPtr<ISpAudio> cpAudio;

	sprintf_s(m_errorMsgBuffer, kErrorMsgBufferSize, "No error set");

    while ( 1 ) 
    {
        // create a recognition engine
        hr = p_recogEngine.CoCreateInstance(CLSID_SpInprocRecognizer);
        if ( FAILED( hr ) )
        {
			//MessageBox(hWnd,"Failed to create a recognition engine\n","Error",MB_OK);
			sprintf_s(m_errorMsgBuffer, kErrorMsgBufferSize, "1 Failed to create a recognition engine\n");
            break;
        }

		if (SUCCEEDED(hr))
        {
            CComPtr<ISpObjectToken> cpAudioToken;
            hr = SpGetDefaultTokenFromCategoryId(SPCAT_AUDIOIN, &cpAudioToken);
            if (SUCCEEDED(hr))
            {
                hr = p_recogEngine->SetInput(cpAudioToken, TRUE);
            }
        }
       
        // create the command recognition context
        hr = p_recogEngine->CreateRecoContext( &p_recogContext );
        if ( FAILED( hr ) )
        {
			//MessageBox(hWnd,"Failed to create the command recognition context\n","Error",MB_OK);
			sprintf_s(m_errorMsgBuffer, kErrorMsgBufferSize, "2 Failed to create the command recognition context");
            break;
        }

        // Let SR know that window we want it to send event information to, and using
        // what message
        hr = p_recogContext->SetNotifyWindowMessage( hWnd, event_id, 0, 0 );
        if ( FAILED( hr ) )
        {
			sprintf_s(m_errorMsgBuffer, kErrorMsgBufferSize, "3 Failed to SetNotifyWindowMessage");
            break;
        }

	    // Tell SR what types of events interest us.  Here we only care about command
        // recognition.
        hr = p_recogContext->SetInterest( SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION) );
        if ( FAILED( hr ) )
        {
			sprintf_s(m_errorMsgBuffer, kErrorMsgBufferSize, "4 Failed to set events");
            break;
        }

        // Load our grammar, which is the compiled form of simple.xml bound into this executable as a
        // user defined ("SRGRAMMAR") resource type.
        hr = p_recogContext->CreateGrammar(grammar_id, &p_grammarObject);
        if (FAILED(hr))
        {
			sprintf_s(m_errorMsgBuffer, kErrorMsgBufferSize, "5 Failed to load grammar");
            break;
        }

		// http://www.geekpedia.com/Thread7939_LoadCmdFromResource-Fails-In-Vista.html
		// http://msdn.microsoft.com/en-us/library/ms723627.aspx
//		LPWSTR res = MAKEINTRESOURCEW(command_resource);
		WORD lang = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
        hr = p_grammarObject->LoadCmdFromResource(NULL, L"IDR_CMD_CFG",
                                                 L"SRGRAMMAR", lang,
                                                 SPLO_DYNAMIC);
		//int prim = PRIMARYLANGID(409);
		//int sub = SUBLANGID(409) ;  
		
		if(FAILED(hr))
		{
			sprintf_s(m_errorMsgBuffer, kErrorMsgBufferSize, "6.1 Failed to load phrases from exe");
			hr = p_grammarObject->LoadCmdFromFile(L"phrases.cfg", SPLO_STATIC);
		}

        if ( FAILED( hr ) )
        {
			sprintf_s(m_errorMsgBuffer, kErrorMsgBufferSize, "6.2 Failed to load resource phrases.cfg");
            break;
        }

        // Set rules to active, we are now listening for commands
        hr = p_grammarObject->SetRuleState(NULL, NULL, SPRS_ACTIVE );
        if ( FAILED( hr ) )
        {
			sprintf_s(m_errorMsgBuffer, kErrorMsgBufferSize, "7 Failed to set listening for commands");
            break;
        }

        break;
    }

    // if we failed and have a partially setup SAPI, close it all down
    if ( FAILED( hr ) )
    {
        g_VoiceRecognition.Deinit();
    }

	m_initialised = SUCCEEDED(hr);
    return ( SUCCEEDED(hr) );
}

void VoiceRecognition::Deinit()
{
	m_initialised = false;

    // Release grammar, if loaded
    if ( p_grammarObject )
    {
        p_grammarObject.Release();
    }
    // Release recognition context, if created
    if ( p_recogContext )
    {
        p_recogContext->SetNotifySink(NULL);
        p_recogContext.Release();
    }
    // Release recognition engine instance, if created
	if ( p_recogEngine )
	{
		p_recogEngine.Release();
	}
}
void VoiceRecognition::ProcessEvent()
{
//	HWND hWnd = (HWND) os_get_window();
    CSpEvent event;  // Event helper class

    // Loop processing events while there are any in the queue
    while (event.GetFrom(p_recogContext) == S_OK)
    {
        // Look at recognition event only
        switch (event.eEventId)
        {
            case SPEI_RECOGNITION:
					g_VoiceRecognition.ExecuteCommand(event.RecoResult());
                break;

        }
    }
}

char *VoiceRecognition::GetLastErrorString()
{
	return m_errorMsgBuffer;
}

// Its not enough to update this, phrases.xml must have this info as well
char *wing_names[] = 
{
		"Alpha",
		"Beta",	
		"Gamma",	
		"Delta",
		"Epsilon",
		"Zeta",
		"Eta",
		"Theta",
		"Iota",
		"Kappa",
		"Lambda",
		"Mu",
		"Nu",
		"Xi",
		"Omicron",
		"Pi",
		"Rho",
		"Sigma",
		"Tau",
		"Upsilon",
		"Phi",
		"Chi",
		"Psi",
		"Omega",
};

/******************************************************************************
* ExecuteCommand *
*----------------*
*   Description:
*       Called to Execute commands that have been identified by the speech engine.
*
******************************************************************************/
extern int button_function(int n);
extern void hud_squadmsg_msg_all_fighters();
extern void hud_squadmsg_shortcut( int command );

char VOICEREC_lastCommand[30];

void VoiceRecognition::ExecuteCommand(ISpPhrase *pPhrase)
{
//	HWND hWnd = (HWND) os_get_window();
    SPPHRASE *pElements;

    // Get the phrase elements, one of which is the rule id we specified in
    // the grammar.  Switch on it to figure out which command was recognized.
    if (SUCCEEDED(pPhrase->GetPhrase(&pElements)))
    {     
		if(DEBUG_ON)
		{
			WCHAR *pwszText;
			pPhrase->GetText((ULONG) SP_GETWHOLEPHRASE, (ULONG) SP_GETWHOLEPHRASE, TRUE, &pwszText, NULL);
			MessageBoxW(NULL,pwszText,NULL,MB_OK);	
		}
		
        switch ( pElements->Rule.ulId )
        {
			case VID_ShipName: 
			{
				int wingType = pElements->pProperties->vValue.ulVal;
				int shipNum = pElements->pProperties->pNextSibling->vValue.ulVal;

				// Cant issue commands to yourself
				if(wingType == 0 && shipNum == 1)
				{
					break;
				}

				const int shipNameStrSize = 20;
				char shipName[shipNameStrSize];
				sprintf_s(shipName,shipNameStrSize, "%s %d", wing_names[wingType], shipNum);

				Msg_instance = ship_name_lookup(shipName);

				if(Msg_instance == -1)
				{
					break;
				}

				if(!(Player->flags & PLAYER_FLAGS_MSG_MODE))
				{
					hud_squadmsg_toggle();
				}

				hud_squadmsg_do_mode(SM_MODE_SHIP_COMMAND);
				break;
			}
			case VID_WingName: 
			{
				int wingType  = pElements->pProperties->vValue.ulVal;
				Msg_instance  = wing_lookup(wing_names[wingType]);

				if(Msg_instance == -1)
				{
					break;
				}

				if(!(Player->flags & PLAYER_FLAGS_MSG_MODE))
				{
					hud_squadmsg_toggle();
				}

				hud_squadmsg_do_mode(SM_MODE_WING_COMMAND);
				break;
			}

			case VID_Action:
			{
				int action = pElements->pProperties->vValue.ulVal;					

				// If menu is up
				if(Player->flags & PLAYER_FLAGS_MSG_MODE)
				{
					switch(action)
					{					
					case VID_DestoryTarget: Msg_shortcut_command = ATTACK_TARGET_ITEM;			break;
					case VID_DisableTarget:	Msg_shortcut_command = DISABLE_TARGET_ITEM;			break;
					case VID_DisarmTarget:	Msg_shortcut_command = DISARM_TARGET_ITEM;			break;
					case VID_DestroySubsys:	Msg_shortcut_command = DISABLE_SUBSYSTEM_ITEM;		break;
					case VID_ProtectTarget:	Msg_shortcut_command = PROTECT_TARGET_ITEM;			break;
					case VID_IgnoreTarget:	Msg_shortcut_command = IGNORE_TARGET_ITEM;			break;
					case VID_FormWing:		Msg_shortcut_command = FORMATION_ITEM;				break;
					case VID_CoverMe:		Msg_shortcut_command = COVER_ME_ITEM;				break;
					case VID_EngageEnemy:	Msg_shortcut_command = ENGAGE_ENEMY_ITEM;			break;
					case VID_Depart:		Msg_shortcut_command = DEPART_ITEM;					break;
					default:				Msg_shortcut_command = -1;							break;

					}

					if(Msg_instance == MESSAGE_ALL_FIGHTERS || Squad_msg_mode == SM_MODE_ALL_FIGHTERS )
					{
					//	nprintf(("warning", "VOICER hud_squadmsg_send_to_all_fighters\n"));
						hud_squadmsg_send_to_all_fighters(Msg_shortcut_command);
					}
					else if(Squad_msg_mode == SM_MODE_SHIP_COMMAND)
					{
					//	nprintf(("warning", "VOICER msg ship %d\n", Msg_instance));
						hud_squadmsg_send_ship_command( Msg_instance, Msg_shortcut_command, 1 );
					}
					else if(Squad_msg_mode == SM_MODE_WING_COMMAND)
					{
					//	nprintf(("warning", "VOICER msg wing %d\n", Msg_instance));
						hud_squadmsg_send_wing_command( Msg_instance, Msg_shortcut_command, 1 );
					}
					else if(Squad_msg_mode == SM_MODE_REINFORCEMENTS )
					{
					}

					hud_squadmsg_toggle();
				}
				else
				{
					// bug these commands should check that they arent greyed out on the HUD before they run
					switch(action)
					{					
					case VID_DestoryTarget: button_function(ATTACK_MESSAGE);				break;
					case VID_DisableTarget:	button_function(DISABLE_MESSAGE);				break;
					case VID_DisarmTarget:	button_function(DISARM_MESSAGE);				break;
					case VID_DestroySubsys:	button_function(ATTACK_SUBSYSTEM_MESSAGE);		break;
					case VID_ProtectTarget:	button_function(PROTECT_MESSAGE);				break;
					case VID_IgnoreTarget:	button_function(IGNORE_MESSAGE);				break;
					case VID_FormWing:		button_function(FORM_MESSAGE);					break;
					case VID_CoverMe:		button_function(COVER_MESSAGE);					break;
					case VID_EngageEnemy:	button_function(ENGAGE_MESSAGE);				break;
					case VID_Depart:		

						button_function(WARP_MESSAGE);					
						break;
					}
				}
		
				break;
			}

			case VID_NonMenu: 
			{
				const int action = pElements->pProperties->vValue.ulVal;
				// switch for actions that shouldnt bring the command window uo
				switch(action)
				{
					case VID_SingleMissiles:
						button_function(CHOOSE_SINGLE_SECONDARY);
						break;
					case VID_DualMissiles:
						button_function(CHOOSE_DUAL_SECONDARY);
						break;
					case VID_CycleMissiles:
						button_function(CYCLE_SECONDARY);
						break;
					case VID_EnableAllGuns:
						button_function(CHOOSE_PRIMARY_ALL_ON);
						break;
					case VID_DisablePrimaryGuns:
						button_function(CHOOSE_PRIMARY_1_OFF);
						break;
					case VID_DisableSecondryGuns:
						button_function(CHOOSE_PRIMARY_2_OFF);
						break;
				}
				break;
			}

			// These commands run no matter what, and will even bring up the menu
			case VID_TopMenu:
			{
				
				const int action = pElements->pProperties->vValue.ulVal;
				bool msgWindow = (Player->flags & PLAYER_FLAGS_MSG_MODE) == PLAYER_FLAGS_MSG_MODE;

				// If the command window is not up, or it is and its a cancel request toggle
				if((msgWindow && action == VID_Cancel) || (!msgWindow && action != VID_Cancel))
				{
					hud_squadmsg_toggle();
				}
				
				switch(action)
				{
					case VID_Ships: 
						hud_squadmsg_do_mode( SM_MODE_SHIP_SELECT );
						break;

					case VID_Wings:
						hud_squadmsg_do_mode( SM_MODE_WING_SELECT );
						break;

					case VID_AllFighters: 
					case VID_AllWings:
					//	Msg_instance = MESSAGE_ALL_FIGHTERS;
					//	Squad_msg_mode == SM_MODE_ALL_FIGHTERS
						hud_squadmsg_msg_all_fighters();

					//	if(Msg_shortcut_command == -1)
					//	{
					//		hud_squadmsg_do_mode( SM_MODE_ALL_FIGHTERS ); 
					//	}
						break;

					case VID_Reinforcements: 
						hud_squadmsg_do_mode( SM_MODE_REINFORCEMENTS );
						break;

					case VID_SupportShip:
						hud_squadmsg_do_mode( SM_MODE_REPAIR_REARM );
						break;

					case VID_AbortSupport:
						hud_squadmsg_do_mode( SM_MODE_REPAIR_REARM_ABORT );
						break;

					case VID_More:
						break;
				}

				break;
			}
        }
        // Free the pElements memory which was allocated for us
        ::CoTaskMemFree(pElements);
    }

}
