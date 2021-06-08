// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/ClientNetLoadActorHelper.h"

#include "Algo/AnyOf.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/ActorSystem.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialSender.h"
#include "Schema/Restricted.h"
#include "SpatialConstants.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SpatialActorUtils.h"

DEFINE_LOG_CATEGORY(LogClientNetLoadActorHelper);

namespace SpatialGDK
{
FClientNetLoadActorHelper::FClientNetLoadActorHelper(USpatialNetDriver& InNetDriver)
{
	NetDriver = &InNetDriver;
}

UObject* FClientNetLoadActorHelper::GetReusableDynamicSubObject(const FUnrealObjectRef ObjectRef)
{
	if (FNetworkGUID* SubObjectNetGUID = GetSavedDynamicSubObjectNetGUID(ObjectRef))
	{
		if (UObject* DynamicSubObject = NetDriver->PackageMap->GetObjectFromNetGUID(*SubObjectNetGUID, /* bIgnoreMustBeMapped */ false))
		{
			NetDriver->PackageMap->ResolveSubobject(DynamicSubObject, ObjectRef);
			UE_LOG(LogClientNetLoadActorHelper, Verbose,
				TEXT("Found reusable dynamic SubObject (ObjectRef offset: %u) for ClientNetLoad actor with entityId %d"),
				ObjectRef.Offset, ObjectRef.Entity);
			return DynamicSubObject;
		}
	}
	return nullptr;
}

void FClientNetLoadActorHelper::EntityRemoved(const Worker_EntityId EntityId, const AActor& Actor)
{
	ClearDynamicSubobjectMetadata(EntityId);
	SaveDynamicSubobjectsMetadata(EntityId, Actor);
}

void FClientNetLoadActorHelper::SaveDynamicSubobjectsMetadata(const Worker_EntityId EntityId, const AActor& Actor)
{
	if (USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(EntityId))
	{
		for (UObject* DynamicSubObject : Channel->CreateSubObjects)
		{
			FNetworkGUID SubObjectNetGUID = NetDriver->PackageMap->GetNetGUIDFromObject(DynamicSubObject);
			if (SubObjectNetGUID.IsValid())
			{
				FUnrealObjectRef SubObjectRef = NetDriver->PackageMap->GetUnrealObjectRefFromNetGUID(SubObjectNetGUID);
				if (SubObjectRef.IsValid() && IsDynamicSubObject(*NetDriver, Actor, SubObjectRef.Offset))
				{
					SaveDynamicSubobjectMetadata(SubObjectRef, SubObjectNetGUID);
					UE_LOG(
						LogClientNetLoadActorHelper, Verbose,
						TEXT("Saved reusable dynamic SubObject ObjectRef (ObjectRef offset: %u) for ClientNetLoad actor with entityId %d"),
						SubObjectRef.Offset, SubObjectRef.Entity);
				}
			}
		}
	}
}

FNetworkGUID* FClientNetLoadActorHelper::GetSavedDynamicSubObjectNetGUID(const FUnrealObjectRef& ObjectRef)
{
	if (TMap<ObjectOffset, FNetworkGUID>* SubobjectOffsetToNetGuid = SpatialEntityRemovedSubobjectMetadata.Find(ObjectRef.Entity))
	{
		if (FNetworkGUID* NetGUID = SubobjectOffsetToNetGuid->Find(ObjectRef.Offset))
		{
			return NetGUID;
		}
	}
	return nullptr;
}

void FClientNetLoadActorHelper::SaveDynamicSubobjectMetadata(const FUnrealObjectRef& ObjectRef, const FNetworkGUID& NetGUID)
{
	TMap<ObjectOffset, FNetworkGUID>& SubobjectOffsetToNetGuid = SpatialEntityRemovedSubobjectMetadata.FindOrAdd(ObjectRef.Entity);
	SubobjectOffsetToNetGuid.Emplace(ObjectRef.Offset, NetGUID);
}

void FClientNetLoadActorHelper::ClearDynamicSubobjectMetadata(const Worker_EntityId InEntityId)
{
	SpatialEntityRemovedSubobjectMetadata.Remove(InEntityId);
}

void FClientNetLoadActorHelper::RemoveRuntimeRemovedComponents(const Worker_EntityId EntityId, const TArray<ComponentData>& NewComponents, AActor& EntityActor)
{
	RemoveDynamicComponentsRemovedByRuntime(EntityId, NewComponents);
	RemoveStaticComponentsRemovedByRuntime(EntityId, NewComponents, EntityActor);
}

void FClientNetLoadActorHelper::RemoveDynamicComponentsRemovedByRuntime(const Worker_EntityId EntityId, const TArray<ComponentData>& NewComponents)
{
	if (TMap<ObjectOffset, FNetworkGUID>* EntityOffsetToNetGuidMap = SpatialEntityRemovedSubobjectMetadata.Find(EntityId))
	{
		for (auto OffsetToNetGuidIterator = EntityOffsetToNetGuidMap->CreateIterator(); OffsetToNetGuidIterator; ++OffsetToNetGuidIterator)
		{
			const ObjectOffset SubobjectOffset = OffsetToNetGuidIterator->Key;
			if (!SubobjectWithOffsetStillExists(NewComponents, SubobjectOffset))
			{
				if (UObject* Object = NetDriver->PackageMap->GetObjectFromNetGUID(OffsetToNetGuidIterator->Value, false /* bIgnoreMustBeMapped */))
				{
					const FUnrealObjectRef EntityObjectRef(EntityId, SubobjectOffset);
					SubobjectRemovedByRuntime(EntityObjectRef, *Object);
				}
				OffsetToNetGuidIterator.RemoveCurrent();
			}
		}
	}
}

void FClientNetLoadActorHelper::RemoveStaticComponentsRemovedByRuntime(const Worker_EntityId EntityId, const TArray<ComponentData>& NewComponents, AActor& EntityActor)
{
	FSubobjectToOffsetMap SubobjectsToOffsets = CreateOffsetMapFromActor(*NetDriver, EntityActor);
	for (auto& SubobjectToOffset : SubobjectsToOffsets)
	{
		UObject& Subobject = *SubobjectToOffset.Key;
		const ObjectOffset Offset = SubobjectToOffset.Value;
		if (SubobjectIsReplicated(Subobject, EntityActor) && !SubobjectWithOffsetStillExists(NewComponents, Offset))
		{
			const FUnrealObjectRef ObjectRef(EntityId, Offset);
			SubobjectRemovedByRuntime(ObjectRef, Subobject);
		}
	}
}

void FClientNetLoadActorHelper::SubobjectRemovedByRuntime(const FUnrealObjectRef& EntityObjectRef, UObject& Subobject)
{
	UE_LOG(LogClientNetLoadActorHelper, Verbose,
	TEXT("A SubObject (ObjectRef offset: %u) on bNetLoadOnClient actor with entityId %d was destroyed while the "
			"actor was out of the client's interest. Destroying the SubObject now."),
		EntityObjectRef.Offset, EntityObjectRef.Entity);
	NetDriver->ActorSystem.Get()->DestroySubObject(EntityObjectRef, Subobject);
}

bool FClientNetLoadActorHelper::SubobjectWithOffsetStillExists(const TArray<ComponentData>& Components,
																const ObjectOffset OffsetToCheckIfContained) const
{
	for (const ComponentData& Component : Components)
	{
		if (Component.GetComponentId() < SpatialConstants::STARTING_GENERATED_COMPONENT_ID)
		{
			continue;
		}

		ObjectOffset NewComponentOffset = 0;
		NetDriver->ClassInfoManager->GetOffsetByComponentId(Component.GetComponentId(), NewComponentOffset);

		if (NewComponentOffset == OffsetToCheckIfContained)
		{
			return true;
		}
	}
	return false;
}

bool FClientNetLoadActorHelper::SubobjectIsReplicated(const UObject& Object, AActor& EntityActor) const
{
	if (const USpatialActorChannel* Channel = NetDriver->GetOrCreateSpatialActorChannel(&EntityActor))
	{
		const bool IsReplicated = Channel->ReplicationMap.Contains(&Object);
		return IsReplicated;
	}
	return false;
}

} // namespace SpatialGDK
