// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialOSEditorToolbarSettings.h"

USpatialOSEditorToolbarSettings::USpatialOSEditorToolbarSettings(const FObjectInitializer& ObjectInitializer) : SpatialOSLaunchArgument(TEXT("default_launch.json")), bStopSpatialOnExit(false), Super(ObjectInitializer)
{
	ProjectRootFolder.Path = FPaths::ConvertRelativePathToFull(FPaths::GetPath(FPaths::GetProjectFilePath()) + FString(TEXT("/../spatial/")));
}

FString USpatialOSEditorToolbarSettings::ToString()
{
	TArray<FStringFormatArg> Args;
	Args.Add(ProjectRootFolder.Path);
	Args.Add(SpatialOSLaunchArgument);
	Args.Add(bStopSpatialOnExit);

	return FString::Format(TEXT("ProjectRootFolder={0}, SpatialOSLaunchArgument={1}, "
								"bStopSpatialOnExit={2}"),
						   Args);
}
