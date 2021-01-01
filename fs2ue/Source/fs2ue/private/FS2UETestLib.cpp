#include "FS2UETestLib.h"
#include "Bmpman/Bmpman.h"
#include "Graphics/GrDummy.h"
#include "Graphics/GrInternal.h"
#include "Ship/Ship.h"

#include "FREESPACE2/Freespace.h"

#include <windows.h> // for MAX_PATH

bool UFS2UETestLib::IsGameInit = false;
UWorld *UFS2UETestLib::TestLibWorld = nullptr;

UFS2UETestLib::UFS2UETestLib(class FObjectInitializer const &Init) : Super(Init)
{

}

void UFS2UETestLib::GameStart(const FString &CommandLine, const UObject *WorldRef)
{
	check(WorldRef);
	TestLibWorld = WorldRef->GetWorld();
	check(TestLibWorld);

	FString DefaultGameDir = "D:\\Games\\Freespace 2\\";

	// Store Unreal game dir
	TCHAR CurrentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, CurrentDir);

	// Set FS2 game dir for init
	TCHAR FreespaceDir[MAX_PATH];
	_tcscpy_s(FreespaceDir, MAX_PATH, DefaultGameDir.GetCharArray().GetData());
	SetCurrentDirectory(FreespaceDir);

	// Get the command line
	char Cmdline[256] = "";
	if (!CommandLine.IsEmpty())
	{
		const TCHAR *CmdlineTCHAR = CommandLine.GetCharArray().GetData();
		wcstombs(Cmdline, CmdlineTCHAR, wcslen(CmdlineTCHAR) + 1);
	}

	IsGameInit = FREESPACE_Init(Cmdline);

	// Restore Unreal game dir
	SetCurrentDirectory(CurrentDir);
}

void UFS2UETestLib::GameTick(const float DeltaTime)
{
	if (IsGameInit)
	{
		FREESPACE_Update(DeltaTime);
	}
}

void UFS2UETestLib::GameEnd()
{
	if (IsGameInit)
	{
		FREESPACE_Shutdown();
		IsGameInit = false;
	}
}

UTexture2D *UFS2UETestLib::LoadTextureTest(const FString &Path)
{
	const TCHAR *TCharPath = Path.GetCharArray().GetData();

	char CharPath[512];
	wcstombs(CharPath, TCharPath, wcslen(TCharPath) + 1);

	int Bitmap = bm_load(CharPath);
	if (Bitmap == -1)
	{
		return nullptr;
	}

	float uScale, vScale;
	return create_texture(Bitmap, TCACHE_TYPE_NORMAL, uScale, vScale);
}

void UFS2UETestLib::LoadShipTest(const FString &Path)
{
	matrix Identity;
	vector Origin;
	ship_create(&Identity, &Origin, 32);
}
