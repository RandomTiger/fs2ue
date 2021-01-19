// Copyright (C) Thomas Liam Whittaker 2017.  All rights reserved. All modified source code herein is the property of Thomas Liam Whittaker. You may not sell or otherwise commercially exploit the source or things you created based on the source. Original source code is owned by Volition, Inc and covered by their copywrite.

#pragma once

#include "GameFramework/GameModeBase.h"
#include "FS2GameMode.generated.h"

class UTexture2D;
class USoundWave;

FS2UE_API DECLARE_LOG_CATEGORY_EXTERN(FS2Code, Log, All);

/**
 * 
 */
UCLASS(Blueprintable)
class FS2UE_API AFS2GameMode : public AGameModeBase
{
	GENERATED_BODY()

	AFS2GameMode();

	UFUNCTION(BlueprintCallable, Category = Game)
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable, Category = Game)
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = Game)
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	static AFS2GameMode* Instance;

	UPROPERTY(EditAnywhere)
	FString CommandLine = "-autoload \"MDH-04\" -forcedummy";

public:
	UFUNCTION(BlueprintCallable, Category = Tests)
	UTexture2D* LoadTexture(const FString& filePath);
	UFUNCTION(BlueprintCallable, Category = Tests)
	USoundWave* GetSoundWaveFromFile(const FString& filePath);
};
