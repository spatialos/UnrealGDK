// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialVirtualWorkerTranslator);

SpatialVirtualWorkerTranslator::SpatialVirtualWorkerTranslator(UAbstractLBStrategy* InLoadBalanceStrategy,
															   PhysicalWorkerName InLocalPhysicalWorkerName)
	: LoadBalanceStrategy(InLoadBalanceStrategy)
	, bIsReady(false)
	, LocalPhysicalWorkerName(InLocalPhysicalWorkerName)
	, LocalVirtualWorkerId(SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
	, LocalPartitionId(SpatialConstants::INVALID_ENTITY_ID)
{
}

const PhysicalWorkerName* SpatialVirtualWorkerTranslator::GetPhysicalWorkerForVirtualWorker(VirtualWorkerId Id) const
{
	if (const SpatialVirtualWorkerTranslator::WorkerInformation* PhysicalWorkerInfo = VirtualToPhysicalWorkerMapping.Find(Id))
	{
		return &PhysicalWorkerInfo->WorkerName;
	}

	return nullptr;
}

Worker_PartitionId SpatialVirtualWorkerTranslator::GetPartitionEntityForVirtualWorker(VirtualWorkerId Id) const
{
	if (const SpatialVirtualWorkerTranslator::WorkerInformation* PhysicalWorkerInfo = VirtualToPhysicalWorkerMapping.Find(Id))
	{
		return PhysicalWorkerInfo->PartitionEntityId;
	}

	return SpatialConstants::INVALID_ENTITY_ID;
}

Worker_EntityId SpatialVirtualWorkerTranslator::GetServerWorkerEntityForVirtualWorker(VirtualWorkerId Id) const
{
	if (const SpatialVirtualWorkerTranslator::WorkerInformation* PhysicalWorkerInfo = VirtualToPhysicalWorkerMapping.Find(Id))
	{
		return PhysicalWorkerInfo->ServerWorkerEntityId;
	}

	return SpatialConstants::INVALID_ENTITY_ID;
}

// The translation schema is a list of Mappings, where each entry has a virtual and physical worker ID.
// This method should only be called on workers who are not authoritative over the mapping and also when
// a worker first becomes authoritative for the mapping.
void SpatialVirtualWorkerTranslator::ApplyMappingFromSchema(Schema_Object* Object)
{
	UE_LOG(LogSpatialVirtualWorkerTranslator, Verbose, TEXT("(%d) Apply valid mapping from schema"), LocalVirtualWorkerId);

	ApplyMappingFromSchema(VirtualToPhysicalWorkerMapping, *Object);

	if (LocalVirtualWorkerId != SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
	{
		return;
	}

	for (const TPair<VirtualWorkerId, WorkerInformation>& Worker : VirtualToPhysicalWorkerMapping)
	{
		const PhysicalWorkerName& WorkerName = Worker.Value.WorkerName;
		const VirtualWorkerId Id = Worker.Key;
		const Worker_PartitionId PartitionEntityId = Worker.Value.PartitionEntityId;
		const Worker_EntityId WorkerEntityId = Worker.Value.ServerWorkerEntityId;

		const Worker_EntityId CurrentWorkerEntityId =
			Cast<USpatialNetDriver>(this->LoadBalanceStrategy->GetWorld()->GetNetDriver())->WorkerEntityId;

		if (WorkerEntityId == CurrentWorkerEntityId)
		{
			LocalVirtualWorkerId = Id;
			LocalPartitionId = PartitionEntityId;
			bIsReady = true;

			// Tell the strategy about the local virtual worker id. This is an "if" and not a "check" to allow unit tests which don't
			// provide a strategy.
			if (LoadBalanceStrategy.IsValid())
			{
				LoadBalanceStrategy->SetLocalVirtualWorkerId(LocalVirtualWorkerId);
			}

			UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("\t-> VirtualWorkerTranslator is now ready for loadbalancing."));
		}
	}
}

void SpatialVirtualWorkerTranslator::ApplyMappingFromSchema(TMap<VirtualWorkerId, WorkerInformation>& VirtualToPhysicalWorkerMapping,
															const Schema_Object& Object)
{
	// Resize the map to accept the new data.
	const uint32 TranslationCount = Schema_GetObjectCount(&Object, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
	VirtualToPhysicalWorkerMapping.Empty(TranslationCount);

	for (uint32 i = 0; i < TranslationCount; i++)
	{
		// Get each entry of the list and then unpack the virtual and physical IDs from the entry.
		Schema_Object* MappingObject =
			Schema_IndexObject(const_cast<Schema_Object*>(&Object), SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID, i);
		const VirtualWorkerId VirtualWorkerId = Schema_GetUint32(MappingObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID);
		const PhysicalWorkerName WorkerName =
			SpatialGDK::GetStringFromSchema(MappingObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME_ID);
		const Worker_EntityId ServerWorkerEntityId = Schema_GetEntityId(MappingObject, SpatialConstants::MAPPING_SERVER_WORKER_ENTITY_ID);
		const Worker_PartitionId PartitionEntityId = Schema_GetEntityId(MappingObject, SpatialConstants::MAPPING_PARTITION_ID);

		// Insert each into the provided map.
		VirtualToPhysicalWorkerMapping.Add(VirtualWorkerId, WorkerInformation{ WorkerName, ServerWorkerEntityId, PartitionEntityId });
	}
}
