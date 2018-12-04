// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SchemaCommandlet.h"
#include "SpatialGDKEditorCommandletPluginPrivate.h"
#include "SpatialGDKEditor.h"

USchemaCommandlet::USchemaCommandlet()
{
	IsClient = false;
	IsEditor = false;
	IsServer = false;
	LogToConsole = true;
}

int32 USchemaCommandlet::Main(const FString& Args)
{

	return 0;
}
