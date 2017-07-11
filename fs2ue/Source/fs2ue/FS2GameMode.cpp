// Copyright (C) Thomas Liam Whittaker 2017.  All rights reserved. All modified source code herein is the property of Thomas Liam Whittaker. You may not sell or otherwise commercially exploit the source or things you created based on the source. Original source code is owned by Volition, Inc and covered by their copywrite.

// Must be first
#include "fs2ue.h"

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

#include "../fs2_source/code/FREESPACE2/Horde.h"
#include "../fs2_source/code/FREESPACE2/Horde.cpp"
#include "../fs2_source/code/FREESPACE2/LevelPaging.h"
#include "../fs2_source/code/FREESPACE2/LevelPaging.cpp"
#include "../fs2_source/code/FREESPACE2/Freespace.h"
#include "../fs2_source/code/FREESPACE2/Freespace.cpp"

// GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle();

/*
//Cmdline_freespace_no_sound = 1;
//Cmdline_freespace_no_music = 1;
*/

#include "FS2GameMode.h"

DEFINE_LOG_CATEGORY(FS2Code);

AFS2GameMode* AFS2GameMode::Instance = nullptr;

AFS2GameMode::AFS2GameMode()
{
	Instance = this;
}

void AFS2GameMode::StartPlay()
{
	Super::StartPlay();
	UE_LOG(FS2Code, Warning, TEXT("StartPlay"));

	PrimaryActorTick.bCanEverTick = true;

	char currentDir[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, currentDir);
	SetCurrentDirectoryA("D:\\Games\\Freespace 2\\");

	char cmdline[128] = "-autoload \"MDH-04\" -forcedummy";
	FREESPACE_Init(cmdline);

	SetCurrentDirectoryA(currentDir);

	UWorld* const World = GetWorld(); // get a reference to the world
}

void AFS2GameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FREESPACE_Update(DeltaTime);
}

void AFS2GameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FREESPACE_Shutdown();

	Super::EndPlay(EndPlayReason);
	UE_LOG(FS2Code, Warning, TEXT("EndPlay"));
}
