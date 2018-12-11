// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SnapshotCommandlet.h"
#include "SpatialGDKEditorCommandletPluginPrivate.h"
#include "SpatialGDKEditor.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

#include "Runtime/Engine/Classes/Engine/ObjectLibrary.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "FileHelpers.h"

USnapshotCommandlet::USnapshotCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 USnapshotCommandlet::Main(const FString& Args)
{
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Snapshot Generation Commandlet Started"));
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Incoming Arguments: %s"), *Args);

	// NOTE: For future use, if snapshot generation configuration at the command line is desired
	// TArray<FString> Tokens;
	// TArray<FString> Switches;
	// TMap<FString, FString> Params;
	// ParseCommandLine(*Args, Tokens, Switches, Params);

	GenerateSnapshots();

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Snapshot Generation Commandlet Complete"));

	return 0;
}

void USnapshotCommandlet::GenerateSnapshots()
{
	FString MapDir = TEXT("/Game");
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Searching %s for maps"), *MapDir);
	TArray<FString> MapFilePaths = GetAllMapPaths(MapDir);
	for (FString MapFilePath : MapFilePaths)
	{
		GenerateSnapshotForMap(MapFilePath);
	}
}

void USnapshotCommandlet::GenerateSnapshotForMap(FString MapPath)
{
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Generating Snapshot for %s"), *MapPath);

	//Load the World
	if (!FEditorFileUtils::LoadMap(MapPath))
	{
		UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Failed to load map %s"), *MapPath);
	}

	//Generate the Snapshot!
	USpatialGDKEditor SpatialGDKEditor;
	SpatialGDKEditor.GenerateSnapshot(
		GWorld, FPaths::SetExtension(FPaths::GetCleanFilename(MapPath), TEXT(".snapshot")),
		FSimpleDelegate::CreateLambda([]() { UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Success!")); }),
		FSimpleDelegate::CreateLambda([]() { UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Failed")); }),
		FSpatialGDKEditorErrorHandler::CreateLambda([](FString ErrorText) { UE_LOG(LogSpatialGDKEditorCommandlet, Error, TEXT("%s"), *ErrorText); }));
}

TArray<FString> USnapshotCommandlet::GetAllMapPaths(FString InMapsPath)
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
