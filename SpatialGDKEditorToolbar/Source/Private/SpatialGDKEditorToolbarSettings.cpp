// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialGDKEditorToolbarSettings.h"

USpatialGDKEditorToolbarSettings::USpatialGDKEditorToolbarSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SpatialOSLaunchConfig(TEXT("default_launch.json"))
	, bStopSpatialOnExit(false)
	, SpatialOSSnapshotFile(GetSpatialOSSnapshotFile())
	, bGenerateSchemaForAllSupportedClasses(false)
{
	ProjectRootFolder.Path = TEXT("");
	SpatialOSSnapshotPath.Path = TEXT("");
	GeneratedSchemaOutputFolder.Path = TEXT("");
}

FString USpatialGDKEditorToolbarSettings::ToString()
{
	TArray<FStringFormatArg> Args;
	Args.Add(ProjectRootFolder.Path);
	Args.Add(SpatialOSLaunchConfig);
	Args.Add(bStopSpatialOnExit);
	Args.Add(SpatialOSSnapshotPath.Path);
	Args.Add(SpatialOSSnapshotFile);
	Args.Add(GeneratedSchemaOutputFolder.Path);
	Args.Add(bGenerateSchemaForAllSupportedClasses);

	return FString::Format(TEXT("ProjectRootFolder={0}, SpatialOSLaunchArgument={1}, "
								"bStopSpatialOnExit={2}, SpatialOSSnapshotPath={3}, "
								"SpatialOSSnapshotFile={4}, GeneratedSchemaOutputFolder={5} "
								"bGenerateSchemaForAllSupportedClasses={6}"),
						   Args);
}
