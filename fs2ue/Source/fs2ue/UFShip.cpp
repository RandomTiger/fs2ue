// Copyright (C) Thomas Liam Whittaker 2017.  All rights reserved. All modified source code herein is the property of Thomas Liam Whittaker. You may not sell or otherwise commercially exploit the source or things you created based on the source. Original source code is owned by Volition, Inc and covered by their copywrite.

#include "UFShip.h"
#include "fs2ue.h"
// Wants to be before windows headers
#include "RuntimeMeshComponent.h"
#include "Providers/RuntimeMeshProviderStatic.h"
#include "Model/FSModel.h"


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

		int count = 0;
		for (int i = 0; i < pm->n_models; i++)
		{
			bsp_info& Submodel = pm->submodel[i];
			const TArray<FModelChunk> &Chunks = Submodel.ModelChucks;
			const int NumChunks = Chunks.Num();

			for (int c = 0; c < NumChunks; c++)
			{
				const FModelChunk& Chunk = Submodel.ModelChucks[c];

				if (Chunk.ueTriangles.Num() == 0)
				{
					continue;
				}

				const int32 SectionIndex = count;
				const int32 MaterialSlot = count;

				extern TMap<int, UTexture2D*> TextureStore;
				int BitmapHandle = Chunk.ueBitmap;
				UTexture2D **TexturePtr = TextureStore.Find(BitmapHandle);
				if (TexturePtr != nullptr)
				{
					UTexture2D *Texture = *TexturePtr;
					check(IsValid(Texture));

					{
						UMaterialInstanceDynamic *MaterialInstance = UMaterialInstanceDynamic::Create(MeshMaterial, nullptr);
						MaterialInstance->SetTextureParameterValue(FName(TEXT("Main")), Texture);
						StaticProvider->SetupMaterialSlot(MaterialSlot, TEXT("TriMat"), MaterialInstance);
					}

					UE_LOG(LogTemp, Warning, TEXT("AShip::AssembleMeshData %s %d %d"), *(Texture->GetName()), Texture->PlatformData->SizeX, Texture->PlatformData->SizeY);
				}

				StaticProvider->CreateSectionFromComponents(
					LODIndex, SectionIndex, MaterialSlot,
					Chunk.ueVertices,
					Chunk.ueTriangles,
					Chunk.ueNormals,
					Chunk.ueTexCoords,
					Chunk.ueColor,
					EmptyTangents,
					ERuntimeMeshUpdateFrequency::Infrequent, bCreateCollision);
				count++;
			}
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
