// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"
#include "Utils/SpatialStatics.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "InterestFactory.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEntityFactory, Log, All);

class AActor;
class UAbstractLBStrategy;
class USpatialActorChannel;
class USpatialNetDriver;
class USpatialPackageMap;
class USpatialClassInfoManager;
class USpatialPackageMapClient;

namespace SpatialGDK
{
class SpatialRPCService;

struct RPCsOnEntityCreation;
using FRPCsOnEntityCreationMap = TMap<TWeakObjectPtr<const UObject>, RPCsOnEntityCreation>;

class SPATIALGDK_API EntityFactory
{
public:
	EntityFactory(USpatialNetDriver* InNetDriver, USpatialPackageMapClient* InPackageMap, USpatialClassInfoManager* InClassInfoManager, SpatialRPCService* InRPCService);

	TArray<FWorkerComponentData> CreateEntityComponents(USpatialActorChannel* Channel, FRPCsOnEntityCreationMap& OutgoingOnCreateEntityRPCs, uint32& OutBytesWritten);
	TArray<FWorkerComponentData> CreateTombstoneEntityComponents(AActor* Actor);

	static TArray<Worker_ComponentId> GetComponentPresenceList(const TArray<FWorkerComponentData>& ComponentDatas);

	static TArray<FWorkerComponentData> CreatePartitionEntityComponents(const Worker_EntityId EntityId, const InterestFactory* InterestFactory, const UAbstractLBStrategy* LbStrategy, VirtualWorkerId VirtualWorker);

	static inline bool IsClientAuthoritativeComponent(Worker_ComponentId ComponentId)
	{
		return ComponentId == SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID
			|| ComponentId == SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY
			|| ComponentId == SpatialConstants::HEARTBEAT_COMPONENT_ID;
	}

private:
	USpatialNetDriver* NetDriver;
	USpatialPackageMapClient* PackageMap;
	USpatialClassInfoManager* ClassInfoManager;
	SpatialRPCService* RPCService;
};
}  // namepsace SpatialGDK
