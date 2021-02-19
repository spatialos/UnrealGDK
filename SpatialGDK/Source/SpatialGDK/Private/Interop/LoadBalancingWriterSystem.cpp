// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/LoadBalancingWriterSystem.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/SubView.h"
namespace SpatialGDK
{
void LoadBalancingWriter::Advance()
{
	const FSubViewDelta& ViewDelta = SubView->GetViewDelta();

	for (const EntityDelta& EntityDelta1 : ViewDelta.EntityDeltas)
	{
		switch (EntityDelta1.Type)
		{
		case EntityDelta::ADD:
		{
			const EntityViewElement& AddedEntity = SubView->GetView()[EntityDelta1.EntityId];
			const ComponentData& LoadBalancingStuffComponent =
				*AddedEntity.Components.FindByPredicate(ComponentIdEquality{ LoadBalancingStuff::ComponentId });
			LoadBalancingActorStuff LBActorStuff(LoadBalancingStuffComponent.GetWorkerComponentData());
			TWeakObjectPtr<UObject> BoundObject = NetDriver->PackageMap->GetObjectFromEntityId(EntityDelta1.EntityId);
			if (ensure(BoundObject.IsValid() && BoundObject->IsA<AActor>()))
			{
				LBActorStuff.Actor = Cast<AActor>(BoundObject.Get());
			}
			DataStore.Add(EntityDelta1.EntityId, LBActorStuff);
		}
		break;
		case EntityDelta::REMOVE:
			DataStore.Remove(EntityDelta1.EntityId);
			break;
		case EntityDelta::UPDATE:
			for (const auto& ComponentUpdate : EntityDelta1.ComponentUpdates)
			{
				if (ComponentUpdate.ComponentId == LoadBalancingData::ComponentId)
				{
					DataStore[EntityDelta1.EntityId].LoadBalancingData = LoadBalancingData(*ComponentUpdate.Data);
				}
			}
			break;
		}
	}

	for (auto& StoredEntityData : DataStore)
	{
		if (SubView->HasAuthority(StoredEntityData.Key, SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID))
		{
			OnActorReplicated(StoredEntityData.Value.Actor.Get());
		}
	}
}

void LoadBalancingWriter::OnActorReplicated(AActor* Actor)
{
	const Worker_EntityId ActorEntityId = NetDriver->PackageMap->GetEntityIdFromObject(Actor);

	if (ActorEntityId == SpatialConstants::INVALID_ENTITY_ID)
	{
		return;
	}

	LoadBalancingActorStuff* LoadBalancingActorStuffPtr = &DataStore.FindOrAdd(ActorEntityId);

	if (!ensure(LoadBalancingActorStuffPtr))
	{
		return;
	}

	LoadBalancingActorStuffPtr->LoadBalancingData = GetOrCreateLoadBalancingData(Actor);

	FWorkerComponentUpdate LoadBalancingUpdate = LoadBalancingActorStuffPtr->LoadBalancingData.CreateLoadBalancingStuffUpdate();
	NetDriver->Connection->SendComponentUpdate(ActorEntityId, &LoadBalancingUpdate);
}

LoadBalancingStuff LoadBalancingWriter::GetOrCreateLoadBalancingData(const AActor* Actor)
{
	LoadBalancingStuff Stuff;
	Stuff.ActorGroupId = NetDriver->LoadBalanceStrategy->GetActorGroupId(*Actor);
	const AActor* Owner = Actor;
	for (; IsValid(Owner->GetOwner()); Owner = Owner->GetOwner())
		;
	check(IsValid(Owner));
	Worker_EntityId OwnerEntityId = NetDriver->PackageMap->GetEntityIdFromObject(Owner);
	check(OwnerEntityId != SpatialConstants::INVALID_ENTITY_ID);
	// Actor Set IDs are implemented as owner entity IDs
	Stuff.ActorSetId = OwnerEntityId;
	return Stuff;
}
} // namespace SpatialGDK