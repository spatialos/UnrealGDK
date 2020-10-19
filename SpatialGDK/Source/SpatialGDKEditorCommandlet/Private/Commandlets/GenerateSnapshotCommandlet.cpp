// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GenerateSnapshotCommandlet.h"
#include "SpatialGDKEditor.h"
#include "SpatialGDKEditorCommandletPrivate.h"

#include "Engine/ObjectLibrary.h"
#include "Engine/World.h"
#include "FileHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"

UGenerateSnapshotCommandlet::UGenerateSnapshotCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UGenerateSnapshotCommandlet::Main(const FString& Args)
{
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Snapshot Generation Commandlet Started"));

	// NOTE: For future use, if snapshot generation configuration at the command line is desired
	// TArray<FString> Tokens;
	// TArray<FString> Switches;
	// TMap<FString, FString> Params;
	// ParseCommandLine(*Args, Tokens, Switches, Params);

	bool bSnapshotGenSuccess = GenerateSnapshots();

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Snapshot Generation Commandlet Complete"));

	return bSnapshotGenSuccess ? 0 : 1;
}

bool UGenerateSnapshotCommandlet::GenerateSnapshots()
{
	FString MapDir = TEXT("/Game");
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Searching %s for maps"), *MapDir);
	TArray<FString> MapFilePaths = GetAllMapPaths(MapDir);
	bool bAllSnapshotGenSuccess = true;
	for (FString MapFilePath : MapFilePaths)
	{
		if (!GenerateSnapshotForMap(MapFilePath))
		{
			bAllSnapshotGenSuccess = false;
			break;
		}
	}
	return bAllSnapshotGenSuccess;
}

bool UGenerateSnapshotCommandlet::GenerateSnapshotForMap(FString MapPath)
{
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Generating Snapshot for %s"), *MapPath);

	// Load the World
	if (!FEditorFileUtils::LoadMap(MapPath))
	{
		UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Failed to load map %s"), *MapPath);
	}

	// Generate the Snapshot!
	bool bSnapshotGenSuccess = false;
	FSpatialGDKEditor SpatialGDKEditor;
	SpatialGDKEditor.GenerateSnapshot(GWorld, FPaths::SetExtension(FPaths::GetCleanFilename(MapPath), TEXT(".snapshot")),
									  FSimpleDelegate::CreateLambda([&bSnapshotGenSuccess]() {
										  UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Success!"));
										  bSnapshotGenSuccess = true;
									  }),
									  FSimpleDelegate::CreateLambda([]() {
										  UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Failed"));
									  }),
									  FSpatialGDKEditorErrorHandler::CreateLambda([](FString ErrorText) {
										  UE_LOG(LogSpatialGDKEditorCommandlet, Error, TEXT("%s"), *ErrorText);
									  }));
	return bSnapshotGenSuccess;
}

TArray<FString> UGenerateSnapshotCommandlet::GetAllMapPaths(FString InMapsPath)
{
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, true);
	ObjectLibrary->LoadAssetDataFromPath(InMapsPath);
	TArray<FAssetData> AssetDatas;
	ObjectLibrary->GetAssetDataList(AssetDatas);
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Found %d maps:"), AssetDatas.Num());

	TArray<FString> Paths = TArray<FString>();
	for (FAssetData& AssetData : AssetDatas)
	{
		FString Path = AssetData.PackageName.ToString();
		Paths.Add(Path);
		UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("\t%s"), *Path);
	}

	return Paths;
}
