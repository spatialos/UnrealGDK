// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"
#include "Utils/SpatialStatics.h"

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
class InterestFactory;
class SpatialRPCService;

struct EntityComponents
{
	TMap<Worker_ComponentId, TUniquePtr<AbstractMutableComponent>> MutableComponents;
	TArray<FWorkerComponentData> ComponentDatas;

	// PRCOMMENT: Hmm, will this incur overhead? Probably...
	void ShiftToComponentDatas()
	{
		for (auto& Pair : MutableComponents)
		{
			ComponentDatas.Add(Pair.Value->CreateComponentData());
		}

		MutableComponents.Empty();
	}
};

class SPATIALGDK_API EntityFactory
{
public:
	EntityFactory(USpatialNetDriver* InNetDriver, USpatialPackageMapClient* InPackageMap, USpatialClassInfoManager* InClassInfoManager,
				  SpatialRPCService* InRPCService);

	static EntityComponents CreateSkeletonEntityComponents(AActor* Actor);
	void WriteUnrealComponents(EntityComponents& EntityComps, USpatialActorChannel* Channel, uint32& OutBytesWritten);
	void WriteLBComponents(EntityComponents& EntityComps, AActor* Actor);
	TArray<FWorkerComponentData> CreateEntityComponents(USpatialActorChannel* Channel, uint32& OutBytesWritten);
	TArray<FWorkerComponentData> CreateTombstoneEntityComponents(AActor* Actor);

	static TArray<FWorkerComponentData> CreatePartitionEntityComponents(const Worker_EntityId EntityId,
																		const InterestFactory* InterestFactory,
																		const UAbstractLBStrategy* LbStrategy,
																		VirtualWorkerId VirtualWorker, bool bDebugContexValid);

	static inline bool IsClientAuthoritativeComponent(Worker_ComponentId ComponentId)
	{
		return ComponentId == SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID || ComponentId == SpatialConstants::HEARTBEAT_COMPONENT_ID;
	}

private:
	USpatialNetDriver* NetDriver;
	USpatialPackageMapClient* PackageMap;
	USpatialClassInfoManager* ClassInfoManager;
	SpatialRPCService* RPCService;
};
} // namespace SpatialGDK
