// Copyright (C) Thomas Liam Whittaker 2017.  All rights reserved. All modified source code herein is the property of Thomas Liam Whittaker. You may not sell or otherwise commercially exploit the source or things you created based on the source. Original source code is owned by Volition, Inc and covered by their copywrite.

#pragma once

//#include "GameFramework/Pawn.h"
//#include "RuntimeMeshActor.h"
#include "FS2UETestLib.generated.h"

class UTexture2D;

UCLASS()
class UFS2UETestLib: public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable)
	static void GameStart(const FString &CommandLine);
	UFUNCTION(BlueprintCallable)
	static void GameEnd();
	UFUNCTION(BlueprintCallable)
	static void GameTick(const float DeltaTime);

	UFUNCTION(BlueprintCallable)
	static UTexture2D *LoadTextureTest(const FString &path);

private:
	static bool IsGameInit;
};

