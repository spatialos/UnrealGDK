// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/ContainerMemoryTracker.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialSender.h"
#include "Interop/SpatialStaticComponentView.h"

DEFINE_LOG_CATEGORY(LogContainerMemoryTracker);

ContainerMemoryTracker::ContainerMemoryTracker(class USpatialNetDriver* InNetDriver)
	: NetDriver(InNetDriver)
{

}

void ContainerMemoryTracker::Tick(float DeltaTime)
{
	AccumulatedTime += DeltaTime;

	if (AccumulatedTime - LastCountMemoryTime > CountMemoryInterval)
	{
		LastCountMemoryTime = AccumulatedTime;

		CountMemory();
	}
}

template <template<typename...> typename ContainerType, typename... T>
void ContainerMemoryTracker::CountContainer(const ContainerType<T...>& Container, FString Identifier, bool& bFirstLog)
{
	uint32 AllocatedSize = Container.GetAllocatedSize();
	if (AllocatedSize > 0 && (!KnownAllocations.Contains(Identifier) || KnownAllocations[Identifier] < AllocatedSize))
	{
		if (bFirstLog)
		{
			FString WorkerId = NetDriver->Connection ? NetDriver->Connection->GetWorkerId() : FString();
			UE_LOG(LogContainerMemoryTracker, Warning, TEXT("!!!%%% MemoryTracker %s:"), *WorkerId);
			bFirstLog = false;
		}

		UE_LOG(LogContainerMemoryTracker, Warning, TEXT("Container [%-40s]: Num: %3d, Allocated size: %8d"), *Identifier, Container.Num(), Container.GetAllocatedSize());
		KnownAllocations.Add(Identifier, AllocatedSize);
	}
}

#define COUNT_CONTAINER(Container) CountContainer(Container, TEXT(#Container), bFirstLog)

void ContainerMemoryTracker::CountMemory()
{
	bool bFirstLog = true;

	COUNT_CONTAINER(NetDriver->SingletonActorChannels);
	COUNT_CONTAINER(NetDriver->EntityToActorChannel);
	COUNT_CONTAINER(NetDriver->QueuedStartupOpLists); // op lists
	COUNT_CONTAINER(NetDriver->DormantEntities);
	COUNT_CONTAINER(NetDriver->PendingDormantChannels);
#if WITH_EDITOR
	COUNT_CONTAINER(NetDriver->TombstonedEntities);
#endif

	if (USpatialReceiver* Receiver = NetDriver->Receiver)
	{
		COUNT_CONTAINER(Receiver->PendingEntitySubobjectDelegations); // subobject attachment auth delegations
		COUNT_CONTAINER(Receiver->ObjectRefToRepStateMap); // set of channel object pairs
		//COUNT_CONTAINER(Receiver->IncomingRPCs);
		COUNT_CONTAINER(Receiver->PendingAddEntities);
		COUNT_CONTAINER(Receiver->PendingAuthorityChanges);
		COUNT_CONTAINER(Receiver->PendingAddComponents); // worker components
		COUNT_CONTAINER(Receiver->QueuedRemoveComponentOps);
		COUNT_CONTAINER(Receiver->PendingActorRequests);
		COUNT_CONTAINER(Receiver->PendingReliableRPCs); // rpc payloads
		COUNT_CONTAINER(Receiver->EntityQueryDelegates);
		COUNT_CONTAINER(Receiver->ReserveEntityIDsDelegates);
		COUNT_CONTAINER(Receiver->CreateEntityDelegates);
		COUNT_CONTAINER(Receiver->AuthorityPlayerControllerConnectionMap);
		COUNT_CONTAINER(Receiver->PendingDynamicSubobjectComponents); // worker components
		COUNT_CONTAINER(Receiver->EntitiesWaitingForAsyncLoad); // arrays of worker components
		COUNT_CONTAINER(Receiver->AsyncLoadingPackages); // arrays of entity ids
	}

	if (USpatialSender* Sender = NetDriver->Sender)
	{
		//COUNT_CONTAINER(NetDriver->Sender->OutgoingRPCs);
		COUNT_CONTAINER(Sender->OutgoingOnCreateEntityRPCs); // rpc payloads
		COUNT_CONTAINER(Sender->RetryRPCs); // rpc payloads
		COUNT_CONTAINER(Sender->UpdatesQueuedUntilAuthorityMap); // component updates
		COUNT_CONTAINER(Sender->ChannelsToUpdatePosition);
		COUNT_CONTAINER(Sender->RPCsToPack); // rpc payloads
	}

	if (USpatialPackageMapClient* PackageMap = NetDriver->PackageMap)
	{
		COUNT_CONTAINER(PackageMap->PendingReferences);
		COUNT_CONTAINER(PackageMap->PendingCreationEntityIds);

		FSpatialNetGUIDCache* GuidCache = (FSpatialNetGUIDCache*)PackageMap->GuidCache.Get();
		COUNT_CONTAINER(GuidCache->NetGUIDToUnrealObjectRef);
		COUNT_CONTAINER(GuidCache->UnrealObjectRefToNetGUID);
	}

	if (USpatialStaticComponentView* StaticView = NetDriver->StaticComponentView)
	{
		COUNT_CONTAINER(StaticView->EntityComponentAuthorityMap); // maps
		COUNT_CONTAINER(StaticView->EntityComponentMap); // component datas
	}
}
