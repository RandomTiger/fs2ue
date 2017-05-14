/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#ifndef UNITY_BUILD
#include <winsock.h>
#include "multi_update.h"
#include "popup.h"
#include "gamesequence.h"
#include "OsRegistry.h"
#include "timer.h"
#include "version.h"
#include "InetGetFile.h"
#include "Cfile.h"
#endif

// -------------------------------------------------------------------------------------------------------------------
// MULTI UPDATE DEFINES/VARS
//

// file get object
InetGetFile *Multi_update_get = NULL;

// set this to be true so that game_shutdown() will fire up the launcher, then post a quit game event
int Multi_update_fireup_launcher_on_exit = 0;

// error code string for convenient reporting
char Multi_update_error_string[512];

// -------------------------------------------------------------------------------------------------------------------
// MULTI UPDATE FUNCTIONS
//

// initialize the http xfer of the version info file, return 1 on success
int multi_update_http_init()
{
	char url_file[512] = "";
	char local_file[512] = "";

	// url
	strcpy(url_file, VERSION_URL);

	// local file
	strcpy(local_file, Cfile_root_dir);
	strcat(local_file, "\\");
	strcat(local_file, VERSION_LOC_FNAME);

	// new file	
	Multi_update_get = new InetGetFile(url_file, local_file);
	if(Multi_update_get == NULL){
		// error string
		strcpy(Multi_update_error_string, XSTR("Could not get data from website", 977));

		return 0;
	}

	return 1;
}

// do frame for the popup. returns 0 if not done yet, 1 if succeeded, 2 on error
int multi_update_http_do()
{	
	// sanity
	if(Multi_update_get == NULL){
		// error string
		strcpy(Multi_update_error_string, XSTR("Could not get data from website", 977));

		return 2;
	}

	// error
	if(Multi_update_get->IsFileError()){			
		delete Multi_update_get;
		Multi_update_get = NULL;

		// error string
		strcpy(Multi_update_error_string, XSTR("Could not get data from website", 977));
		
		return 2;
	} 	

	// done!
	if(Multi_update_get->IsFileReceived()){
		delete Multi_update_get;
		Multi_update_get = NULL;
		return 1;
	}
	
	// connecting, receiving
	if(Multi_update_get->IsConnecting() || Multi_update_get->IsReceiving()){
		return 0;
	}

	return 0;
}

// close down the http xfer
void multi_update_http_close()
{	
}

// error verifying, prompt the user for some action
int multi_update_error_verifying()
{
	char out_str[512];

	memset(out_str, 0, 512);
	strcpy(out_str, "(");
	strcat(out_str, Multi_update_error_string);
	strcat(out_str, ")\n\n");
	strcat(out_str, XSTR("There was an error verifying your version of Freespace, if you continue, you will not necessarily be up to date", 978));	

	switch(popup(PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON, 2, XSTR("&Go back", 1524), XSTR("&Continue", 1525), out_str)){
	// continue on in freespace like nothing happened
	case 1:
	case -1:
		return MULTI_UPDATE_CONTINUE;

	// go back to the main menu
	default:
		return MULTI_UPDATE_MAIN_MENU;
	}

	// should never happen, but if so, move back to the main menu
	return MULTI_UPDATE_MAIN_MENU;
}

// check to see if the version of FS on this machine is not recent. run in a popup
// if the versions don't check out, bail to the launcher
// returns 0 if freespace should continue, 1 if freespace should go back to the main menu,
// and 2 if the "shutdown" event was posted
int multi_update_gobaby()
{			
	char msg[512] = "";
	int ret_code;
	int my_code = MULTI_UPDATE_MAIN_MENU;

	// maybe skip
	if(os_config_read_uint(NULL, "SkipVerify", 0)){
		return MULTI_UPDATE_CONTINUE;
	}

	memset(Multi_update_error_string, 0, 512);	

	// initialize the http connection
	if(!multi_update_http_init()){		
		return multi_update_error_verifying();
	}

	// run the popup
	extern char Multi_options_proxy[512];
	extern ushort Multi_options_proxy_port;
	if(strlen(Multi_options_proxy) > 0){
		sprintf(msg, "%s (%s : %d)", XSTR("Verifying Freespace Version",981), Multi_options_proxy, Multi_options_proxy_port);
	} else {
		strcpy(msg, XSTR("Verifying Freespace Version",981));
	}
	ret_code = popup_till_condition(multi_update_http_do, XSTR("Cancel",948), msg);		

	// determine what to do now
	switch(ret_code){
	// success
	case 1 : 
		// compare the versions
		switch(version_compare(VERSION_LOC_FNAME, NULL, NULL, NULL, NULL, NULL, NULL)){
		// error
		case -1:
			my_code = multi_update_error_verifying();
			break;

		// earlier version - need to update
		case 0:			
			switch(popup(PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON, 2, "&Update Later", "&Yes", XSTR("A new version of Freespace is available. You must update to the new version to play on PXO\n\nAuto update now?", 980))){
			// update later (go back to main hall for now
			case 0 :
			case -1:
				my_code = MULTI_UPDATE_MAIN_MENU;			
				break;

			default:
				// set things up so that the launcher is launched when Freespace is done closing
				Multi_update_fireup_launcher_on_exit = 1;
				gameseq_post_event(GS_EVENT_QUIT_GAME);

				my_code = MULTI_UPDATE_SHUTTING_DOWN;			
			}				
			break;

		// same version or higher
		case 1:
		case 2:
			my_code = MULTI_UPDATE_CONTINUE;
			break;
		}
		break;

	// error of some kind (should probably notify the user or something)
	case 2:	
		my_code = multi_update_error_verifying();		
		break;

	// this should never happen, but if it does, go back to the main menu
	default :
		my_code = multi_update_error_verifying();		
		break;
	}	

	// close down
	multi_update_http_close();
	
	return my_code;		
}

