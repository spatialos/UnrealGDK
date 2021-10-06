// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialVirtualWorkerTranslationManager.h"

#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialOSDispatcherInterface.h"
#include "SpatialConstants.h"
#include "Utils/EntityFactory.h"
#include "Utils/InterestFactory.h"
#include "Utils/SchemaUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialVirtualWorkerTranslationManager);

SpatialVirtualWorkerTranslationManager::SpatialVirtualWorkerTranslationManager(SpatialOSWorkerInterface* InConnection,
																			   USpatialNetDriver* InNetDriver,
																			   SpatialVirtualWorkerTranslator* InTranslator)
	: Translator(InTranslator)
	, Connection(InConnection)
	, NetDriver(InNetDriver)
	, Partitions({})
	, bWorkerEntityQueryInFlight(false)
{
}

void SpatialVirtualWorkerTranslationManager::SetNumberOfVirtualWorkers(const uint32 InNumVirtualWorkers)
{
	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log, TEXT("TranslationManager is configured to look for %d workers"),
		   InNumVirtualWorkers);

	NumVirtualWorkers = InNumVirtualWorkers;

	// Currently, this should only be called once on startup. In the future we may allow for more flexibility.
	VirtualWorkersToAssign.Reserve(NumVirtualWorkers);

	for (uint32 i = 1; i <= NumVirtualWorkers; i++)
	{
		VirtualWorkersToAssign.Emplace(i);
	}
}

void SpatialVirtualWorkerTranslationManager::Advance(const TArray<Worker_Op>& Ops)
{
	CommandsHandler.ProcessOps(Ops);
}

const SpatialVirtualWorkerTranslationManager::FVirtualToPhysicalWorkerMapping&
SpatialVirtualWorkerTranslationManager::GetVirtualWorkerMapping() const
{
	return VirtualToPhysicalWorkerMapping;
}
