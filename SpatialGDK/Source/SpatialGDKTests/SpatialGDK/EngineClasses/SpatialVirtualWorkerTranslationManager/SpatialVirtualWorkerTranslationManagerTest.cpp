// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "SpatialConstants.h"
#include "SpatialOSDispatcherMock.h"
#include "SpatialOSWorkerConnectionMock.h"
#include "Utils/SchemaUtils.h"
#include "UObject/UObjectGlobals.h"

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_schema.h>

#define VIRTUALWORKERTRANSLATIONMANAGER_TEST(TestName) \
	GDK_TEST(Core, SpatialVirtualWorkerTranslationManager, TestName)

VIRTUALWORKERTRANSLATIONMANAGER_TEST(Given_a_test_THEN_pass)
{
	TUniquePtr<SpatialOSWorkerConnectionMock> Connection = MakeUnique<SpatialOSWorkerConnectionMock>();
	TUniquePtr<SpatialOSDispatcherMock> Dispatcher = MakeUnique<SpatialOSDispatcherMock>();
	TUniquePtr<SpatialVirtualWorkerTranslationManager> Manager = MakeUnique<SpatialVirtualWorkerTranslationManager>(Dispatcher.Get(), Connection.Get(), nullptr);

	Worker_AuthorityChangeOp Op;
	Op.entity_id = SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID;
	Op.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	Op.authority = WORKER_AUTHORITY_AUTHORITATIVE;

	Manager->AuthorityChanged(Op);

	TestTrue("On gaining authority, the TranslationManager queried for worker entities.", Connection->GetLastEntityQuery() != nullptr);

	return true;
}

