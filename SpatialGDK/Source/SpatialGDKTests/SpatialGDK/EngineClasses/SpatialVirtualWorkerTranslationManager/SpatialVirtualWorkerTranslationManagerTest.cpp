// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "Utils/SchemaUtils.h"
#include "UObject/UObjectGlobals.h"

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_schema.h>

#define VIRTUALWORKERTRANSLATIONMANAGER_TEST(TestName) \
	GDK_TEST(Core, SpatialVirtualWorkerTranslationManager, TestName)

VIRTUALWORKERTRANSLATIONMANAGER_TEST(Given_a_test_THEN_pass)
{
 	TUniquePtr<SpatialVirtualWorkerTranslationManager> Manager = MakeUnique<SpatialVirtualWorkerTranslationManager>(nullptr, nullptr, nullptr);

	return true;
}

