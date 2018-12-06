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
	Args.Add(GeneratedSchemaOutputFolder.Path);
	Args.Add(bGenerateSchemaForAllSupportedClasses);
	return FString::Format(TEXT(
		"SpatialOSLaunchArgument={0}, "
		"GeneratedSchemaOutputFolder={1}, "
		"bGenerateSchemaForAllSupportedClasses={2}")
		, Args);
}

