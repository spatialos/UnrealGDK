// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialGDKEditorSettings.h"

USpatialGDKEditorSettings::USpatialGDKEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SpatialOSDirectory.Path = GetSpatialOSDirectory();
	SpatialOSLaunchConfig = GetSpatialOSLaunchConfig();
	SpatialOSSnapshotPath.Path = GetSpatialOSSnapshotPath();
	SpatialOSSnapshotFile = GetSpatialOSSnapshotFile();
	GeneratedSchemaOutputFolder.Path = GetGeneratedSchemaOutputFolder();
}

FString USpatialGDKEditorSettings::ToString()
{
	TArray<FStringFormatArg> Args;
	Args.Add(SpatialOSDirectory.Path);
	Args.Add(SpatialOSLaunchConfig);
	Args.Add(bStopSpatialOnExit);
	Args.Add(SpatialOSSnapshotPath.Path);
	Args.Add(SpatialOSSnapshotFile);
	Args.Add(GeneratedSchemaOutputFolder.Path);
	Args.Add(GetSpatialOSCommandLineLaunchFlags());

	return FString::Format(TEXT(
		"ProjectRootFolder={0}, "
		"SpatialOSLaunchArgument={1}, "
		"bStopSpatialOnExit={2}, "
		"SpatialOSSnapshotPath={3}, "
		"SpatialOSSnapshotFile={4}, "
		"GeneratedSchemaOutputFolder={5}, "
		"SpatialOSCommandLineLaunchFlags={6}")
		, Args);
}

