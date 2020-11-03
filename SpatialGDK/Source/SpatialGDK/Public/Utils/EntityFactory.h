// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"
#include "Utils/SpatialStatics.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

DECLARE_LOG_CATEGORY_EXTERN(LogEntityFactory, Log, All);

class AActor;
class USpatialActorChannel;
class USpatialNetDriver;
class USpatialPackageMap;
class USpatialClassInfoManager;
class USpatialPackageMapClient;

namespace SpatialGDK
{
class SpatialRPCService;

struct RPCsOnEntityCreation;
using FRPCsOnEntityCreationMap = TMap<TWeakObjectPtr<const UObject>, RPCsOnEntityCreation, FDefaultSetAllocator,
									  TWeakObjectPtrMapKeyFuncs<TWeakObjectPtr<const UObject>, SpatialGDK::RPCsOnEntityCreation, false>>;

struct EntityComponents
{
	TMap<Worker_ComponentId, TUniquePtr<DataComponent>> ModifiableComponents;
	TMap<Worker_ComponentId, FWorkerComponentData> ComponentsToDelegateToAuthoritativeWorker;
	TArray<FWorkerComponentData> ComponentDatas;

	// PRCOMMENT: Hmm, will this incur overhead? Probably...
	void ShiftToComponentDatas()
	{
		for (auto& Pair : ModifiableComponents)
		{
			ComponentDatas.Add(Pair.Value->CreateComponentData());
		}

		for (auto& Pair : ComponentsToDelegateToAuthoritativeWorker)
		{
			ComponentDatas.Add(Pair.Value);
		}

		ModifiableComponents.Empty();
	}
};

class SPATIALGDK_API EntityFactory
{
public:
	EntityFactory(USpatialNetDriver* InNetDriver, USpatialPackageMapClient* InPackageMap, USpatialClassInfoManager* InClassInfoManager,
				  SpatialRPCService* InRPCService);

	static EntityComponents CreateSkeletonEntityComponents(AActor* Actor);
	void WriteUnrealComponents(EntityComponents& EntityComps, USpatialActorChannel* Channel,
							   FRPCsOnEntityCreationMap& OutgoingOnCreateEntityRPCs, uint32& OutBytesWritten);
	void WriteLBComponents(EntityComponents& EntityComps, AActor* Actor);
	TArray<FWorkerComponentData> CreateEntityComponents(USpatialActorChannel* Channel, FRPCsOnEntityCreationMap& OutgoingOnCreateEntityRPCs,
														uint32& OutBytesWritten);
	TArray<FWorkerComponentData> CreateTombstoneEntityComponents(AActor* Actor);

	static TArray<Worker_ComponentId> GetComponentPresenceList(const TArray<FWorkerComponentData>& ComponentDatas);

private:
	USpatialNetDriver* NetDriver;
	USpatialPackageMapClient* PackageMap;
	USpatialClassInfoManager* ClassInfoManager;
	SpatialRPCService* RPCService;
};
} // namespace SpatialGDK
