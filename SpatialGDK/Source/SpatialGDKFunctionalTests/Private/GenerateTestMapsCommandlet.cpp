// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GenerateTestMapsCommandlet.h"
#include "TestMapGeneration.h"
#include "TestMaps/GeneratedTestMap.h"

DEFINE_LOG_CATEGORY(LogGenerateTestMapsCommandlet);

UGenerateTestMapsCommandlet::UGenerateTestMapsCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UGenerateTestMapsCommandlet::Main(const FString& CmdLineParams)
{
	UE_LOG(LogGenerateTestMapsCommandlet, Display, TEXT("Generate test maps commandlet started."));

	SpatialGDK::TestMapGeneration::GenerateTestMaps();

	// Success
	return 0;
}
