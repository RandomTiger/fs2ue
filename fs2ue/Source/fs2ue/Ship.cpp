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

#undef UpdateResource
#pragma optimize("", off)
void AShip::AssembleMeshData(const polymodel * const pm)
{
	URuntimeMeshProviderStatic* StaticProvider = NewObject<URuntimeMeshProviderStatic>(this, TEXT("RuntimeMeshProvider"));
	if (StaticProvider)
	{
		// The static provider should initialize before we use it
		GetRuntimeMeshComponent()->Initialize(StaticProvider);

		TArray<FRuntimeMeshTangent> EmptyTangents;

		const int32 LODIndex = 0;
		const bool bCreateCollision = false;

		for (int i = 0; i < pm->n_models; i++)
		{
			const int32 SectionIndex = i;
			const int32 MaterialSlot = i;

			extern TMap<int, UTexture2D*> TextureStore;
			int BitmapHandle = pm->submodel[i].ueBitmap;
			UTexture2D **TexturePtr = TextureStore.Find(BitmapHandle);
			if (TexturePtr != nullptr)
			{
				UTexture2D *Texture = *TexturePtr;
				if (IsValid(Texture))
				{
					UMaterialInstanceDynamic *MaterialInstance = UMaterialInstanceDynamic::Create(MeshMaterial, nullptr);
					MaterialInstance->SetTextureParameterValue(FName(TEXT("Main")), Texture);
					StaticProvider->SetupMaterialSlot(MaterialSlot, TEXT("TriMat"), MaterialInstance);
				}
			}

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

void AShip::BeginPlay()
{
	Super::BeginPlay();
}

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
#pragma optimize("", off)