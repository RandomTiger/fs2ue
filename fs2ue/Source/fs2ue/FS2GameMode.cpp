// Copyright (C) Thomas Liam Whittaker 2017.  All rights reserved. All modified source code herein is the property of Thomas Liam Whittaker. You may not sell or otherwise commercially exploit the source or things you created based on the source. Original source code is owned by Volition, Inc and covered by their copywrite.

// Must be first
#include "FS2GameMode.h"

#include "fs2ue.h"
#include "FS2UETestLib.h"

#include "FREESPACE2/Freespace.h"

// GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle();

// D:\dev\OldDepots\P4WorkspaceShuttle\Freespace\Game\Ogre for model load

/*
//Cmdline_freespace_no_sound = 1;
//Cmdline_freespace_no_music = 1;
*/

DEFINE_LOG_CATEGORY(FS2Code);

AFS2GameMode* AFS2GameMode::Instance = nullptr;

AFS2GameMode::AFS2GameMode()
{
	Instance = this;
}

void AFS2GameMode::BeginPlay()
{
	Super::BeginPlay();

	PrimaryActorTick.bCanEverTick = true;
	UFS2UETestLib::GameStart(CommandLine, GetWorld());
}

void AFS2GameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UFS2UETestLib::GameTick(DeltaTime);
}

void AFS2GameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UFS2UETestLib::GameEnd();
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
		sw->SetSampleRate(*WaveInfo.pSamplesPerSec);
		sw->NumChannels = *WaveInfo.pChannels;
		sw->RawPCMDataSize = WaveInfo.SampleDataSize;
		sw->SoundGroup = ESoundGroup::SOUNDGROUP_Default;
	}
	else {
		return nullptr;
	}

	return sw;
}

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
