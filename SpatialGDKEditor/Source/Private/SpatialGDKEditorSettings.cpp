// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialGDKEditorSettings.h"

USpatialGDKEditorSettings::USpatialGDKEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bGenerateSchemaForAllSupportedClasses(true)
{
	SpatialOSDirectory.Path = TEXT("");
	GeneratedSchemaOutputFolder.Path = TEXT("");
}
