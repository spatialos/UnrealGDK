// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestingSchemaHelpers.h"

#include "CoreMinimal.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_worker.h>

Schema_Object* TestingSchemaHelpers::CreateTranslationComponentDataFields()
{
	Worker_ComponentData Data = {};
	Data.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	Data.schema_type = Schema_CreateComponentData();
	return Schema_GetComponentDataFields(Data.schema_type);
}

void TestingSchemaHelpers::AddTranslationComponentDataMapping(Schema_Object* ComponentDataFields, VirtualWorkerId VWId,
															  const PhysicalWorkerName& WorkerName)
{
	Schema_Object* SchemaObject = Schema_AddObject(ComponentDataFields, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
	Schema_AddUint32(SchemaObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID, VWId);
	SpatialGDK::AddStringToSchema(SchemaObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME, WorkerName);
}
