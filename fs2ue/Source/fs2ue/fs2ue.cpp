/*
* Copyright (C) Thomas Liam Whittaker 2017.  All rights reserved.
*
* All modified source code herein is the property of Thomas Liam Whittaker. You may not sell
* or otherwise commercially exploit the source or things you created based on the
* source. Original source code is owned by Volition, Inc and covered by their copywrite.
*
*/

#include "fs2ue.h"
#include "../Test.h"
#include "../Test.cpp"

#include "../fs2_source/code/TomLib/src/StateMachine/StateMachine.h"
#include "../fs2_source/code/TomLib/src/StateMachine/StateMachine.cpp"

#include "../fs2_source/code/TomLib/src/ComponentSystem/ComponentSystem.h"
#include "../fs2_source/code/TomLib/src/ComponentSystem/ComponentSystem.cpp"

#include "../fs2_source/code/UnityBuild.h"
#include "../fs2_source/code/UnityBuild.cpp"

#include "../fs2_source/code/UnityBuildD3D.h"
#include "../fs2_source/code/UnityBuildD3D.cpp"
#if !defined(FS2_UE)
#include "../fs2_source/code/UnityBuildD3D9.h"
#include "../fs2_source/code/UnityBuildD3D9.cpp"
#endif

//#include "../fs2_source/code/Sound/Sound.cpp"
//#include "../fs2_source/code/Sound/AudioStr.h"
//#include "../fs2_source/code/Sound/AudioStr.cpp"

#include "../fs2_source/code/FREESPACE2/Horde.h"
#include "../fs2_source/code/FREESPACE2/Horde.cpp"
#include "../fs2_source/code/FREESPACE2/LevelPaging.h"
#include "../fs2_source/code/FREESPACE2/LevelPaging.cpp"
#include "../fs2_source/code/FREESPACE2/Freespace.h"
#include "../fs2_source/code/FREESPACE2/Freespace.cpp"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, fs2ue, "fs2ue" );

Test test;

// GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle();

/*
//Cmdline_freespace_no_sound = 1;
//Cmdline_freespace_no_music = 1;

// -window -autoload "MDH-04" -forcedummy
*/