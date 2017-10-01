// Copyright (C) Thomas Liam Whittaker 2017.  All rights reserved. All modified source code herein is the property of Thomas Liam Whittaker. You may not sell or otherwise commercially exploit the source or things you created based on the source. Original source code is owned by Volition, Inc and covered by their copywrite.

#pragma once

#include "GameFramework/Pawn.h"
#include "RuntimeMeshComponent.h"
#include "Ship.generated.h"

struct polymodel;

UCLASS()
class FS2UE_API AShip : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AShip(const FObjectInitializer& ObjectInitializer);

	void AssembleMeshData(const polymodel * const pm);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	float DebugRadius;
private:
	UBillboardComponent* Billboard;
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test")
	URuntimeMeshComponent *RuntimeMesh;
	
};
