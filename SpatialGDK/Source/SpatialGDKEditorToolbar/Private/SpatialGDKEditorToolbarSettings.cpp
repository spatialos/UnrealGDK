// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialGDKEditorToolbarSettings.h"

USpatialGDKEditorToolbarSettings::USpatialGDKEditorToolbarSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SpatialOSLaunchConfig(TEXT("default_launch.json"))
	, bStopSpatialOnExit(false)
	, SpatialOSSnapshotFile(GetSpatialOSSnapshotFile())
{
}

FString USpatialGDKEditorToolbarSettings::ToString()
{
	FString BaseString = Super::ToString();

	TArray<FStringFormatArg> Args;
	Args.Add(SpatialOSLaunchConfig);
	Args.Add(bStopSpatialOnExit);
	Args.Add(SpatialOSSnapshotFile);

	FString ToolbarString = FString::Format(TEXT(
		"ProjectRootFolder={0}, "
		"bStopSpatialOnExit={1}, "
		"SpatialOSSnapshotFile={2}"),
		Args);

	return BaseString + TEXT(", ") + ToolbarString;
}
