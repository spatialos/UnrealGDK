// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialGDKEditorSettings.h"

USpatialGDKEditorSettings::USpatialGDKEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bGenerateSchemaForAllSupportedClasses(true)
{
	SpatialOSDirectory.Path = TEXT("");
	GeneratedSchemaOutputFolder.Path = TEXT("");
}

FString USpatialGDKEditorSettings::ToString()
{
	TArray<FStringFormatArg> Args;
	Args.Add(SpatialOSDirectory.Path);
	Args.Add(SpatialOSSnapshotPath.Path);
	Args.Add(GeneratedSchemaOutputFolder.Path);
	Args.Add(bGenerateSchemaForAllSupportedClasses);

	return FString::Format(TEXT(
		"SpatialOSLaunchArgument={0}, "
		"SpatialOSSnapshotPath={1}, "
		"GeneratedSchemaOutputFolder={2}, "
		"bGenerateSchemaForAllSupportedClasses={3}")
		, Args);
}

