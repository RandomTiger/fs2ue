// Copyright (C) Thomas Liam Whittaker 2017.  All rights reserved. All modified source code herein is the property of Thomas Liam Whittaker. You may not sell or otherwise commercially exploit the source or things you created based on the source. Original source code is owned by Volition, Inc and covered by their copywrite.

#pragma once

#include "GameFramework/Pawn.h"
#include "RuntimeMeshActor.h"
#include "Ship.generated.h"

struct polymodel;
class URuntimeMeshComponent;

UCLASS()
class FS2UE_API AShip : public ARuntimeMeshActor
{
	GENERATED_BODY()

public:
	AShip(const FObjectInitializer& ObjectInitializer);

	virtual void OnConstruction(const FTransform& Transform) override;

	void AssembleMeshData(const polymodel * const pm);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	float DebugRadius;
private:
	UPROPERTY()
	UBillboardComponent *Billboard = nullptr;
	UPROPERTY()
	UMaterial *MeshMaterial = nullptr;
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test")
	URuntimeMeshComponent *RuntimeMesh = nullptr;
	
};
