// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialCommonTypes.h"

#include <WorkerSDK/improbable/c_schema.h>

struct SPATIALGDK_API TestingSchemaHelpers
{
	// Can be used to create a Schema_Object to be passed to VirtualWorkerTranslator.
	static Schema_Object* CreateTranslationComponentDataFields();
	// Can be used to add a mapping between virtual work id and physical worker name.
	static void AddTranslationComponentDataMapping(Schema_Object* ComponentDataFields, VirtualWorkerId VWId,
												   const PhysicalWorkerName& WorkerName);
};
