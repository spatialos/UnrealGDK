// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "SpatialConstants.h"
#include "SpatialGDKTests/SpatialGDK/Interop/Connection/SpatialOSWorkerInterface/SpatialOSWorkerConnectionSpy.h"
#include "SpatialGDKTests/SpatialGDK/Interop/SpatialOSDispatcherInterface/SpatialOSDispatcherSpy.h"
#include "Utils/SchemaUtils.h"
#include "UObject/UObjectGlobals.h"

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_schema.h>

#define VIRTUALWORKERTRANSLATIONMANAGER_TEST(TestName) \
	GDK_TEST(Core, SpatialVirtualWorkerTranslationManager, TestName)
namespace
{

// Given a TranslationManager, Dispatcher, and Connection, give the TranslationManager authority
// so that it registers a QueryDelegate with the Dispatcher Mock, then query for that Delegate
// and return it so that tests can focus on the Delegate's correctness.
EntityQueryDelegate* SetupQueryDelegateTests(SpatialVirtualWorkerTranslationManager* Manager, SpatialOSDispatcherSpy* Dispatcher, SpatialOSWorkerConnectionSpy* Connection)
{
	// Build an authority change op which gives the worker authority over the translation.
	Worker_AuthorityChangeOp QueryOp;
	QueryOp.entity_id = SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID;
	QueryOp.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	QueryOp.authority = WORKER_AUTHORITY_AUTHORITATIVE;

	// Let the Manager know it should have authority. This should trigger an EntityQuery and register a response delegate.
	Manager->AuthorityChanged(QueryOp);
	Worker_RequestId EntityQueryRequestId = Connection->GetLastRequestId();
	EntityQueryDelegate* Delegate = Dispatcher->GetEntityQueryDelegate(EntityQueryRequestId);
	Connection->ClearLastEntityQuery();

	return Delegate;
}

}  // anonymous namespace

VIRTUALWORKERTRANSLATIONMANAGER_TEST(Given_an_authority_change_THEN_query_for_worker_entities_when_appropriate)
{
	TUniquePtr<SpatialOSWorkerConnectionSpy> Connection = MakeUnique<SpatialOSWorkerConnectionSpy>();
	TUniquePtr<SpatialOSDispatcherSpy> Dispatcher = MakeUnique<SpatialOSDispatcherSpy>();
	TUniquePtr<SpatialVirtualWorkerTranslationManager> Manager = MakeUnique<SpatialVirtualWorkerTranslationManager>(Dispatcher.Get(), Connection.Get(), nullptr);

	// Build an authority change op which gives the worker authority over the translation.
	Worker_AuthorityChangeOp QueryOp;
	QueryOp.entity_id = SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID;
	QueryOp.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
	QueryOp.authority = WORKER_AUTHORITY_AUTHORITATIVE;

	Manager->AuthorityChanged(QueryOp);
	TestTrue("On gaining authority, the TranslationManager queried for server worker entities.", Connection->GetLastEntityQuery() != nullptr);

	EntityQueryDelegate* Delegate = Dispatcher->GetEntityQueryDelegate(0);
	TestTrue("An EntityQueryDelegate was added to the dispatcher when the query was made", Delegate != nullptr);

	Connection->ClearLastEntityQuery();
	Manager->AuthorityChanged(QueryOp);
	TestTrue("TranslationManager doesn't make a second query if one is in flight.", Connection->GetLastEntityQuery() == nullptr);

	return true;
}

VIRTUALWORKERTRANSLATIONMANAGER_TEST(Given_a_failed_query_response_THEN_query_again)
{
	TUniquePtr<SpatialOSWorkerConnectionSpy> Connection = MakeUnique<SpatialOSWorkerConnectionSpy>();
	TUniquePtr<SpatialOSDispatcherSpy> Dispatcher = MakeUnique<SpatialOSDispatcherSpy>();
	TUniquePtr<SpatialVirtualWorkerTranslator> Translator = MakeUnique<SpatialVirtualWorkerTranslator>(nullptr, SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME);
	TUniquePtr<SpatialVirtualWorkerTranslationManager> Manager = MakeUnique<SpatialVirtualWorkerTranslationManager>(Dispatcher.Get(), Connection.Get(), Translator.Get());

	EntityQueryDelegate* Delegate = SetupQueryDelegateTests(Manager.Get(), Dispatcher.Get(), Connection.Get());

	Worker_EntityQueryResponseOp ResponseOp;
	ResponseOp.status_code = WORKER_STATUS_CODE_TIMEOUT;
	ResponseOp.result_count = 0;
	ResponseOp.message = "Failed call";

	Manager->SetLayerVirtualWorkerMapping(1);

	Delegate->ExecuteIfBound(ResponseOp);
	TestTrue("After a failed query response, the TranslationManager queried again for server worker entities.", Connection->GetLastEntityQuery() != nullptr);

	return true;
}

VIRTUALWORKERTRANSLATIONMANAGER_TEST(Given_a_successful_query_without_enough_workers_THEN_query_again)
{
	TUniquePtr<SpatialOSWorkerConnectionSpy> Connection = MakeUnique<SpatialOSWorkerConnectionSpy>();
	TUniquePtr<SpatialOSDispatcherSpy> Dispatcher = MakeUnique<SpatialOSDispatcherSpy>();
	TUniquePtr<SpatialVirtualWorkerTranslator> Translator = MakeUnique<SpatialVirtualWorkerTranslator>(nullptr, SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME);
	TUniquePtr<SpatialVirtualWorkerTranslationManager> Manager = MakeUnique<SpatialVirtualWorkerTranslationManager>(Dispatcher.Get(), Connection.Get(), Translator.Get());

	EntityQueryDelegate* Delegate = SetupQueryDelegateTests(Manager.Get(), Dispatcher.Get(), Connection.Get());

	Worker_EntityQueryResponseOp ResponseOp;
	ResponseOp.status_code = WORKER_STATUS_CODE_SUCCESS;
	ResponseOp.result_count = 0;
	ResponseOp.message = "Successfully returned 0 entities";

	// Make sure the TranslationManager is expecting more workers than are returned.
	Manager->SetLayerVirtualWorkerMapping(1);

	Delegate->ExecuteIfBound(ResponseOp);
	TestTrue("When not enough workers available, the TranslationManager queried again for server worker entities.", Connection->GetLastEntityQuery() != nullptr);

	return true;
}

VIRTUALWORKERTRANSLATIONMANAGER_TEST(Given_a_successful_query_with_invalid_workers_THEN_query_again)
{
	TUniquePtr<SpatialOSWorkerConnectionSpy> Connection = MakeUnique<SpatialOSWorkerConnectionSpy>();
	TUniquePtr<SpatialOSDispatcherSpy> Dispatcher = MakeUnique<SpatialOSDispatcherSpy>();
	TUniquePtr<SpatialVirtualWorkerTranslator> Translator = MakeUnique<SpatialVirtualWorkerTranslator>(nullptr, SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME);
	TUniquePtr<SpatialVirtualWorkerTranslationManager> Manager = MakeUnique<SpatialVirtualWorkerTranslationManager>(Dispatcher.Get(), Connection.Get(), Translator.Get());

	EntityQueryDelegate* Delegate = SetupQueryDelegateTests(Manager.Get(), Dispatcher.Get(), Connection.Get());

	// This is an invalid entity to be returned, because it doesn't have a "Worker" component on it.
	Worker_Entity worker;
	worker.entity_id = 1001;
	worker.component_count = 0;

	Worker_EntityQueryResponseOp ResponseOp;
	ResponseOp.status_code = WORKER_STATUS_CODE_SUCCESS;
	ResponseOp.result_count = 0;
	ResponseOp.message = "Successfully returned 0 entities";
	ResponseOp.results = &worker;

	// Make sure the TranslationManager is only expecting a single worker.
	Manager->SetLayerVirtualWorkerMapping(1);

	Delegate->ExecuteIfBound(ResponseOp);
	TestTrue("When enough workers available but they are invalid, the TranslationManager queried again for server worker entities.", Connection->GetLastEntityQuery() != nullptr);

	return true;
}
