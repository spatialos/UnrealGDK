// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialGDKEditorToolbarSettings.h"

USpatialGDKEditorToolbarSettings::USpatialGDKEditorToolbarSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SpatialOSLaunchConfig(TEXT("default_launch.json"))
	, bStopSpatialOnExit(false)
	, SpatialOSSnapshotFile(GetSpatialOSSnapshotFile())
	, bGenerateSchemaForAllSupportedClasses(true)
{
	SpatialOSSnapshotPath.Path = TEXT("");
	GeneratedSchemaOutputFolder.Path = TEXT("");
}

FString USpatialGDKEditorToolbarSettings::ToString()
{
	TArray<FStringFormatArg> Args;
	Args.Add(SpatialOSLaunchConfig);
	Args.Add(bStopSpatialOnExit);
	Args.Add(SpatialOSSnapshotPath.Path);
	Args.Add(SpatialOSSnapshotFile);
	Args.Add(GeneratedSchemaOutputFolder.Path);
	Args.Add(bGenerateSchemaForAllSupportedClasses);

	return FString::Format(TEXT("SpatialOSLaunchArgument={0}, bStopSpatialOnExit={1}, "
								"SpatialOSSnapshotPath={2}, SpatialOSSnapshotFile={3}, "
								"GeneratedSchemaOutputFolder={4}, bGenerateSchemaForAllSupportedClasses={5}"),
						   Args);
}
