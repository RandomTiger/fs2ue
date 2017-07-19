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
	IsInit = false;
}

void AFS2GameMode::StartPlay()
{
	Super::StartPlay();

	PrimaryActorTick.bCanEverTick = true;

	if (EnableFS2)
	{
		// Store Unreal game dir
		TCHAR currentDir[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, currentDir);

		// Set FS2 game dir for init
		TCHAR fs2dir[MAX_PATH];
		_tcscpy_s(fs2dir, MAX_PATH, DefaultGameDir.GetCharArray().GetData());
		SetCurrentDirectory(fs2dir);

		// Get the command line
		char cmdline[128];
		TCHAR *cmdlineTCHAR = CommandLine.GetCharArray().GetData();
		wcstombs(cmdline, cmdlineTCHAR, wcslen(cmdlineTCHAR) + 1);

		IsInit = FREESPACE_Init(cmdline);

		// Restore Unreal game dir
		SetCurrentDirectory(currentDir);
	}

	UWorld* const World = GetWorld(); // get a reference to the world
}

void AFS2GameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsInit)
	{
		FREESPACE_Update(DeltaTime);
	}
}

void AFS2GameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsInit)
	{
		FREESPACE_Shutdown();
		IsInit = false;
	}

	Super::EndPlay(EndPlayReason);
}

//https://answers.unrealengine.com/questions/264233/is-it-possible-to-dynamically-import-files-ie-wav.html
USoundWave* AFS2GameMode::GetSoundWaveFromFile(const FString& filePath)
{
	USoundWave* sw = NewObject<USoundWave>(USoundWave::StaticClass());

	if (!sw)
		return nullptr;

	TArray < uint8 > rawFile;

	FFileHelper::LoadFileToArray(rawFile, filePath.GetCharArray().GetData());
	FWaveModInfo WaveInfo;

	if (WaveInfo.ReadWaveInfo(rawFile.GetData(), rawFile.Num()))
	{
		sw->InvalidateCompressedData();

		sw->RawData.Lock(LOCK_READ_WRITE);
		void* LockedData = sw->RawData.Realloc(rawFile.Num());
		FMemory::Memcpy(LockedData, rawFile.GetData(), rawFile.Num());
		sw->RawData.Unlock();

		int32 DurationDiv = *WaveInfo.pChannels * *WaveInfo.pBitsPerSample * *WaveInfo.pSamplesPerSec;
		if (DurationDiv)
		{
			sw->Duration = *WaveInfo.pWaveDataSize * 8.0f / DurationDiv;
		}
		else
		{
			sw->Duration = 0.0f;
		}
		sw->SampleRate = *WaveInfo.pSamplesPerSec;
		sw->NumChannels = *WaveInfo.pChannels;
		sw->RawPCMDataSize = WaveInfo.SampleDataSize;
		sw->SoundGroup = ESoundGroup::SOUNDGROUP_Default;
	}
	else {
		return nullptr;
	}

	return sw;
}

#undef UpdateResource

// https://forums.unrealengine.com/showthread.php?92360-Runtime-texture-and-normal-map
UTexture2D* AFS2GameMode::LoadTexture(const FString& filePath)
{
	int width = 256;
	int height = 256;

	UTexture2D* texture = UTexture2D::CreateTransient(width, height, PF_B8G8R8A8);

	int32 *designTexData = (int32 *) texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	for (int h = 0; h < height; h++)
	{
		for (int w = 0; w < width; w++)
		{
			designTexData[w + h * width] = 0xFF00FFFF;
		}
	}
	
	texture->PlatformData->Mips[0].BulkData.Unlock();
	texture->UpdateResource();

	return texture;
}
