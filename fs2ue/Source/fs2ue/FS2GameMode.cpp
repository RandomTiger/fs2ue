// Copyright (C) Thomas Liam Whittaker 2017.  All rights reserved. All modified source code herein is the property of Thomas Liam Whittaker. You may not sell or otherwise commercially exploit the source or things you created based on the source. Original source code is owned by Volition, Inc and covered by their copywrite.

#include "fs2ue.h"
#include "FS2GameMode.h"

DEFINE_LOG_CATEGORY(FS2Code);

void AFS2GameMode::StartPlay()
{
	Super::StartPlay();
	UE_LOG(FS2Code, Warning, TEXT("StartPlay"));

	PrimaryActorTick.bCanEverTick = true;
}

void AFS2GameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UE_LOG(FS2Code, Warning, TEXT("Tick"));
}

void AFS2GameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	UE_LOG(FS2Code, Warning, TEXT("EndPlay"));
}