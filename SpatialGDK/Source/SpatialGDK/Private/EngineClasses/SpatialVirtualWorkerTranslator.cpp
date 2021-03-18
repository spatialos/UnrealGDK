// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialVirtualWorkerTranslator);

SpatialVirtualWorkerTranslator::SpatialVirtualWorkerTranslator(UAbstractLBStrategy* InLoadBalanceStrategy, USpatialNetDriver* InNetDriver,
															   PhysicalWorkerName InLocalPhysicalWorkerName)
	: NetDriver(InNetDriver)
	, LoadBalanceStrategy(InLoadBalanceStrategy)
	, bIsReady(false)
	, LocalPhysicalWorkerName(InLocalPhysicalWorkerName)
	, LocalVirtualWorkerId(SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
	, LocalPartitionId(SpatialConstants::INVALID_ENTITY_ID)
{
}

const PhysicalWorkerName* SpatialVirtualWorkerTranslator::GetPhysicalWorkerForVirtualWorker(VirtualWorkerId Id) const
{
	if (const SpatialGDK::VirtualWorkerInfo* PhysicalWorkerInfo = VirtualToPhysicalWorkerMapping.Find(Id))
	{
		return &PhysicalWorkerInfo->PhysicalWorkerName;
	}

	return nullptr;
}

Worker_PartitionId SpatialVirtualWorkerTranslator::GetPartitionEntityForVirtualWorker(VirtualWorkerId Id) const
{
	if (const SpatialGDK::VirtualWorkerInfo* PhysicalWorkerInfo = VirtualToPhysicalWorkerMapping.Find(Id))
	{
		return PhysicalWorkerInfo->PartitionId;
	}

	return SpatialConstants::INVALID_ENTITY_ID;
}

Worker_EntityId SpatialVirtualWorkerTranslator::GetServerWorkerEntityForVirtualWorker(VirtualWorkerId Id) const
{
	if (const SpatialGDK::VirtualWorkerInfo* PhysicalWorkerInfo = VirtualToPhysicalWorkerMapping.Find(Id))
	{
		return PhysicalWorkerInfo->ServerWorkerEntity;
	}

	return SpatialConstants::INVALID_ENTITY_ID;
}

void SpatialVirtualWorkerTranslator::ApplyVirtualWorkerManagerData(const SpatialGDK::VirtualWorkerTranslation& Translation)
{
	UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("ApplyVirtualWorkerManagerData for %s:"), *LocalPhysicalWorkerName);

	VirtualToPhysicalWorkerMapping.Empty(Translation.VirtualWorkerMapping.Num());
	for (auto& VirtualWorkerInfo : Translation.VirtualWorkerMapping)
	{
		UpdateMapping(VirtualWorkerInfo);
	}

#if !NO_LOGGING
	if (LoadBalanceStrategy.IsValid() && LoadBalanceStrategy->IsReady())
	{
		UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("\t-> Strategy: %s"), *LoadBalanceStrategy->ToString());

		for (const auto& Entry : VirtualToPhysicalWorkerMapping)
		{
			UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("\t-> Assignment: Virtual Worker %d to %s with server worker entity: %lld"),
				   Entry.Key, *(Entry.Value.PhysicalWorkerName), Entry.Value.ServerWorkerEntity);
		}
	}
#endif //!NO_LOGGING
}

void SpatialVirtualWorkerTranslator::UpdateMapping(const SpatialGDK::VirtualWorkerInfo& VirtualWorkerInfo)
{
	VirtualToPhysicalWorkerMapping.Add(VirtualWorkerInfo.VirtualWorkerId, VirtualWorkerInfo);

	if (LocalVirtualWorkerId == SpatialConstants::INVALID_VIRTUAL_WORKER_ID && VirtualWorkerInfo.PhysicalWorkerName == LocalPhysicalWorkerName)
	{
		LocalVirtualWorkerId = VirtualWorkerInfo.VirtualWorkerId;
		LocalPartitionId = VirtualWorkerInfo.PartitionId;
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
