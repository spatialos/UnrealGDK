// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditor.h"

#include "Async.h"
#include "SpatialGDKEditorSchemaGenerator.h"

#include "Editor.h"

#include "AssetRegistryModule.h"
#include "GeneralProjectSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKEditor);

USpatialGDKEditor::USpatialGDKEditor()
	: bSchemaGeneratorRunning(false)
{
}

void USpatialGDKEditor::GenerateSchema(FSimpleDelegate SuccessCallback, FSimpleDelegate FailureCallback, FSpatialGDKEditorErrorHandler ErrorCallback)
{
	if (bSchemaGeneratorRunning)
	{
		UE_LOG(LogSpatialGDKEditor, Warning, TEXT("Schema generation is already running"));
		return;
	}

	bSchemaGeneratorRunning = true;

	// Force spatial networking so schema layouts are correct
	UGeneralProjectSettings* GeneralProjectSettings = GetMutableDefault<UGeneralProjectSettings>();
	bool bCachedSpatialNetworking = GeneralProjectSettings->bSpatialNetworking;
	GeneralProjectSettings->bSpatialNetworking = true;

	// Ensure all our spatial classes are loaded into memory before running
	CacheSpatialObjects(SPATIALCLASS_GenerateTypeBindings, ErrorCallback);

	SchemaGeneratorResult = Async<bool>(EAsyncExecution::Thread, SpatialGDKGenerateSchema,
		[this, bCachedSpatialNetworking, SuccessCallback, FailureCallback]()
	{
		if (!SchemaGeneratorResult.IsReady() || SchemaGeneratorResult.Get() != true)
		{
			if (FailureCallback.IsBound())
			{
				FailureCallback.Execute();
			}
		}
		else
		{
			if (SuccessCallback.IsBound())
			{
				SuccessCallback.Execute();
			}
		}
		GetMutableDefault<UGeneralProjectSettings>()->bSpatialNetworking = bCachedSpatialNetworking;
		bSchemaGeneratorRunning = false;
	});
}

void USpatialGDKEditor::GenerateSnapshot(UWorld* World, FString SnapshotFilename, FSimpleDelegate SuccessCallback, FSimpleDelegate FailureCallback, FSpatialGDKEditorErrorHandler ErrorCallback)
{
	// Ensure all our singletons are loaded into memory before running
	CacheSpatialObjects(SPATIALCLASS_Singleton, ErrorCallback);

	const bool bSuccess = SpatialGDKGenerateSnapshot(World, SnapshotFilename);

	if (bSuccess)
	{
		if (SuccessCallback.IsBound())
		{
			SuccessCallback.Execute();
		}
	}
	else
	{
		if (FailureCallback.IsBound())
		{
			FailureCallback.Execute();
		}
	}
}

void USpatialGDKEditor::CacheSpatialObjects(uint32 SpatialFlags, FSpatialGDKEditorErrorHandler ErrorCallback)
{
	// Load the asset registry module
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Before running the schema generator, ensure all blueprint classes that have been tagged with 'spatial' are loaded
	TArray<FAssetData> AssetData;
	uint32 SpatialClassFlags = 0;
	AssetRegistry.GetAssetsByClass(UBlueprint::StaticClass()->GetFName(), AssetData, true);
	for (auto& It : AssetData)
	{
		if (It.GetTagValue("SpatialClassFlags", SpatialClassFlags))
		{
			if (SpatialClassFlags & SpatialFlags)
			{
				FString ObjectPath = It.ObjectPath.ToString() + TEXT("_C");
				UClass* LoadedClass = LoadObject<UClass>(nullptr, *ObjectPath, nullptr, LOAD_EditorOnly, nullptr);
				UE_LOG(LogSpatialGDKEditor, Log, TEXT("Found spatial blueprint class `%s`."), *ObjectPath);
				if (LoadedClass == nullptr)
				{
					if (ErrorCallback.IsBound())
					{
						ErrorCallback.Execute(FString::Printf(TEXT("Error: Failed to load blueprint %s."), *ObjectPath));
					}
				}
			}
		}
	}
}
