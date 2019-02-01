// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GenerateSchemaAndSnapshotsCommandlet.h"
#include "SpatialGDKEditorCommandletPrivate.h"
#include "SpatialGDKEditor.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/ObjectLibrary.h"
#include "Engine/World.h"
#include "FileHelpers.h"
#include "Misc/Paths.h"
#include "Engine/LevelStreaming.h"

UGenerateSchemaAndSnapshotsCommandlet::UGenerateSchemaAndSnapshotsCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UGenerateSchemaAndSnapshotsCommandlet::Main(const FString& Args)
{
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema & Snapshot Generation Commandlet Started"));

	TArray<FString> Tokens;
	TArray<FString> Switches;
	TMap<FString, FString> Params;
	ParseCommandLine(*Args, Tokens, Switches, Params);


	FSpatialGDKEditor SpatialGDKEditor;
	if (Params.Contains(TEXT("MapName")))
		GenerateSchemaAndSnapshotForMap(SpatialGDKEditor, *Params.Find(TEXT("MapName")));
	else
	{
	}

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema & Snapshot Generation Commandlet Complete"));

	return 0;
}

void UGenerateSchemaAndSnapshotsCommandlet::GenerateSchemaAndSnapshotForMap(FSpatialGDKEditor& InSpatialGDKEditor, FString InMapName)
{
	//Load persistent Level (this will load over any previously loaded levels)
	FString MapPath = TEXT("/Game");
	if (!InMapName.StartsWith(TEXT("/")))
	{
		MapPath += "/";
	}
	MapPath += InMapName;
	if (!FEditorFileUtils::LoadMap(MapPath))	//This loads the world into GWorld -run=GenerateSchemaAndSnapshots -MapName="ThirdPersonCPP/Maps/ThirdPersonExampleMap"
	{
		UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Failed to load map %s"), *MapPath);
	}

	//Ensure all sub-levels are also loaded, recursively
	const TArray<ULevelStreaming*> StreamingLevels = GWorld->GetStreamingLevels();
	for (ULevelStreaming* StreamingLevel : StreamingLevels)
	{
		FLatentActionInfo LatentInfo;
		UGameplayStatics::LoadStreamLevel(GWorld, StreamingLevel->GetWorldAssetPackageFName(), false, true, LatentInfo);
	}

	//Generate Schema Iteration
	GenerateSchemaForLoadedMap(InSpatialGDKEditor);

	//Generate Snapshot
	GenerateSnapshotForLoadedMap(InSpatialGDKEditor, FPaths::GetCleanFilename(MapPath));
}

void UGenerateSchemaAndSnapshotsCommandlet::GenerateSchemaForLoadedMap(FSpatialGDKEditor& InSpatialGDKEditor)
{
	InSpatialGDKEditor.GenerateSchema(
		FSimpleDelegate::CreateLambda([]() { UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Generation Completed!")); }),
		FSimpleDelegate::CreateLambda([]() { UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Generation Failed")); }),
		FSpatialGDKEditorErrorHandler::CreateLambda([](FString ErrorText) { UE_LOG(LogSpatialGDKEditorCommandlet, Error, TEXT("%s"), *ErrorText); }));
	while (InSpatialGDKEditor.IsSchemaGeneratorRunning())
		FPlatformProcess::Sleep(0.1f);
}

void UGenerateSchemaAndSnapshotsCommandlet::GenerateSnapshotForLoadedMap(FSpatialGDKEditor& InSpatialGDKEditor, FString MapName)
{
	//Generate the Snapshot!
	InSpatialGDKEditor.GenerateSnapshot(
		GWorld, FPaths::SetExtension(FPaths::GetCleanFilename(MapName), TEXT(".snapshot")),
		FSimpleDelegate::CreateLambda([]() { UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Success!")); }),
		FSimpleDelegate::CreateLambda([]() { UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Failed")); }),
		FSpatialGDKEditorErrorHandler::CreateLambda([](FString ErrorText) { UE_LOG(LogSpatialGDKEditorCommandlet, Error, TEXT("%s"), *ErrorText); }));
}
void UGenerateSchemaAndSnapshotsCommandlet::GenerateSchema(FSpatialGDKEditor& SpatialGDKEditor)
{
	SpatialGDKEditor.GenerateSchema(
		FSimpleDelegate::CreateLambda([]() { UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Generation Completed!")); }),
		FSimpleDelegate::CreateLambda([]() { UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Generation Failed")); }),
		FSpatialGDKEditorErrorHandler::CreateLambda([](FString ErrorText) { UE_LOG(LogSpatialGDKEditorCommandlet, Error, TEXT("%s"), *ErrorText); }));
	while (SpatialGDKEditor.IsSchemaGeneratorRunning())
		FPlatformProcess::Sleep(0.1f);
}

void UGenerateSchemaAndSnapshotsCommandlet::GenerateSnapshots(FSpatialGDKEditor& SpatialGDKEditor)
{
	FString MapDir = TEXT("/Game");
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Searching %s for maps"), *MapDir);
	TArray<FString> MapFilePaths = GetAllMapPaths(MapDir);
	for (FString MapFilePath : MapFilePaths)
	{
		GenerateSnapshotForMap(SpatialGDKEditor, MapFilePath);
	}
}

TArray<FString> UGenerateSchemaAndSnapshotsCommandlet::GetAllMapPaths(FString InMapsPath)
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

void UGenerateSchemaAndSnapshotsCommandlet::GenerateSnapshotForMap(FSpatialGDKEditor& SpatialGDKEditor, FString MapPath)
{
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Generating Snapshot for %s"), *MapPath);

	//Load the World
	if (!FEditorFileUtils::LoadMap(MapPath))
	{
		UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Failed to load map %s"), *MapPath);
	}

	//Generate the Snapshot!
	SpatialGDKEditor.GenerateSnapshot(
		GWorld, FPaths::SetExtension(FPaths::GetCleanFilename(MapPath), TEXT(".snapshot")),
		FSimpleDelegate::CreateLambda([]() { UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Success!")); }),
		FSimpleDelegate::CreateLambda([]() { UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Failed")); }),
		FSpatialGDKEditorErrorHandler::CreateLambda([](FString ErrorText) { UE_LOG(LogSpatialGDKEditorCommandlet, Error, TEXT("%s"), *ErrorText); }));
}
