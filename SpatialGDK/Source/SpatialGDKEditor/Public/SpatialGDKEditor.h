// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Async/Future.h"
#include "CoreMinimal.h"
#include "UObject/StrongObjectPtr.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKEditor, Log, All);

DECLARE_DELEGATE_OneParam(FSpatialGDKEditorErrorHandler, FString);

class FSpatialGDKPackageAssembly;

class SPATIALGDKEDITOR_API FSpatialGDKEditor
{
public:
	FSpatialGDKEditor();

	bool GenerateSchema(bool bFullScan);
	void GenerateSnapshot(UWorld* World, FString SnapshotFilename, FSimpleDelegate SuccessCallback, FSimpleDelegate FailureCallback, FSpatialGDKEditorErrorHandler ErrorCallback);
	void LaunchCloudDeployment(FSimpleDelegate SuccessCallback, FSimpleDelegate FailureCallback);
	void StopCloudDeployment(FSimpleDelegate SuccessCallback, FSimpleDelegate FailureCallback);

	bool IsSchemaGeneratorRunning() { return bSchemaGeneratorRunning; }
	bool FullScanRequired();

	TSharedRef<FSpatialGDKPackageAssembly> GetPackageAssemblyRef();

private:
	bool bSchemaGeneratorRunning;
	TFuture<bool> SchemaGeneratorResult;
	TFuture<bool> LaunchCloudResult;
	TFuture<bool> StopCloudResult;

	bool LoadPotentialAssets(TArray<TStrongObjectPtr<UObject>>& OutAssets);

	FDelegateHandle OnAssetLoadedHandle;
	void OnAssetLoaded(UObject* Asset);
	void RemoveEditorAssetLoadedCallback();

	TSharedRef<FSpatialGDKPackageAssembly>SpatialGDKPackageAssemblyInstance;
};
