// Copyright (C) Thomas Liam Whittaker 2017.  All rights reserved. All modified source code herein is the property of Thomas Liam Whittaker. You may not sell or otherwise commercially exploit the source or things you created based on the source. Original source code is owned by Volition, Inc and covered by their copywrite.

#include "fs2ue.h"
#include "Ship.h"
#include "../fs2_source/code/UnityBuild.h"

// Sets default values
AShip::AShip(const FObjectInitializer& ObjectInitializer)
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
/*
	Billboard = ObjectInitializer.CreateDefaultSubobject<UBillboardComponent>(this, TEXT("Billboard"));
	Billboard->Mobility = EComponentMobility::Type::Movable;
	RootComponent = Billboard;
*/
	RuntimeMesh = ObjectInitializer.CreateDefaultSubobject<URuntimeMeshComponent>(this, TEXT("RuntimeMesh"));
	RuntimeMesh->Mobility = EComponentMobility::Type::Movable;
	RootComponent = RuntimeMesh;
}

void AShip::AssembleMeshData(const polymodel * const pm)
{
	for (int i = 0; i < pm->n_models; i++)
	{
		RuntimeMesh->CreateMeshSection(i, pm->submodel[i].Vertices, pm->submodel[i].Triangles, false, EUpdateFrequency::Infrequent);
	}
}

// Called when the game starts or when spawned
void AShip::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AShip::Tick(float DeltaTime)
{
	int32 segs = 8;

	float scaleUp = 1.0f;

	FVector pos = GetActorLocation();
	
	DrawDebugSphere(
		GetWorld(),
		pos * scaleUp,
		DebugRadius * scaleUp,
		segs,
		FColor(255, 0, 0)
	);

	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AShip::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

