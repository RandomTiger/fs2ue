// Copyright (C) Thomas Liam Whittaker 2017.  All rights reserved. All modified source code herein is the property of Thomas Liam Whittaker. You may not sell or otherwise commercially exploit the source or things you created based on the source. Original source code is owned by Volition, Inc and covered by their copywrite.

#include "fs2ue.h"
#include "Ship.h"
// Wants to be before windows headers
#include "RuntimeMeshComponent.h"
#include "Providers/RuntimeMeshProviderStatic.h"

#include "../fs2_source/code/UnityBuild.h"

AShip::AShip(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	SetMobility(EComponentMobility::Type::Movable);

	static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("Material'/Game/FSCore/Materials/UnlitMaterial'"));
	if (Material.Object != NULL)
	{
		MeshMaterial = (UMaterial*)Material.Object;
	}
}

void AShip::AssembleMeshData(const polymodel * const pm)
{
	URuntimeMeshProviderStatic* StaticProvider = NewObject<URuntimeMeshProviderStatic>(this, TEXT("RuntimeMeshProvider"));
	if (StaticProvider)
	{
		// The static provider should initialize before we use it
		GetRuntimeMeshComponent()->Initialize(StaticProvider);

		StaticProvider->SetupMaterialSlot(0, TEXT("TriMat"), MeshMaterial);

		// This creates 3 positions for a triangle
		TArray<FVector> Positions{ FVector(0, -50, 0), FVector(0, 0, 100), FVector(0, 50, 0) };

		// This creates 3 vertex colors
		TArray<FColor> Colors{ FColor::Blue, FColor::Red, FColor::Green };

		// This indexes our simple triangle
		TArray<int32> Triangles = { 0, 1, 2 };

		TArray<FVector> EmptyNormals;
		TArray<FVector2D> EmptyTexCoords;
		TArray<FRuntimeMeshTangent> EmptyTangents;

		const int32 LODIndex = 0;
		const int32 MaterialSlot = 0;
		const bool bCreateCollision = false;

		for (int i = 0; i < pm->n_models; i++)
		{
			const int32 SectionIndex = i;

			StaticProvider->CreateSectionFromComponents(
				LODIndex, SectionIndex, MaterialSlot,
				pm->submodel[i].ueVertices, 
				pm->submodel[i].ueTriangles, 
				pm->submodel[i].ueNormals, 
				pm->submodel[i].ueTexCoords,
				pm->submodel[i].ueColor,
				EmptyTangents, 
				ERuntimeMeshUpdateFrequency::Infrequent, bCreateCollision);
		}
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
	Super::Tick(DeltaTime);

	int32 segs = 8;
	float scaleUp = 1.0f;
	FVector pos = GetActorLocation();
	
	DrawDebugSphere(
		GetWorld(),
		pos * scaleUp,
		DebugRadius * scaleUp,
		segs,
		FColor(255, 0, 0));
}