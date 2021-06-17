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
				   TEXT("Found reusable dynamic SubObject (ObjectRef offset: %u) for ClientNetLoad actor with entityId %s"),
				   ObjectRef.Offset, *ObjectRef.Entity.ToString());
			return DynamicSubObject;
		}
	}
	return nullptr;
}

void FClientNetLoadActorHelper::EntityRemoved(const FSpatialEntityId EntityId, const AActor& Actor)
{
	ClearDynamicSubobjectMetadata(EntityId);
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
						TEXT("Saved reusable dynamic SubObject ObjectRef (ObjectRef offset: %u) for ClientNetLoad actor with entityId %s"),
						SubObjectRef.Offset, *SubObjectRef.Entity.ToString());
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

void FClientNetLoadActorHelper::ClearDynamicSubobjectMetadata(const FSpatialEntityId InEntityId)
{
	SpatialEntityRemovedSubobjectMetadata.Remove(InEntityId);
}

void FClientNetLoadActorHelper::RemoveRuntimeRemovedComponents(const FSpatialEntityId EntityId, const TArray<ComponentData>& NewComponents)
{
	if (TMap<ObjectOffset, FNetworkGUID>* SubobjectOffsetToNetGuid = SpatialEntityRemovedSubobjectMetadata.Find(EntityId))
	{
		// Go over each stored sub-object and determine whether it is contained within the new components array
		// If it is not contained within the new components array, it means the sub-object was removed while out of the client's interest
		// If so, remove it now
		for (auto OffsetToNetGuidIterator = SubobjectOffsetToNetGuid->CreateIterator(); OffsetToNetGuidIterator; ++OffsetToNetGuidIterator)
		{
			const ObjectOffset ObjectOffset = OffsetToNetGuidIterator->Key;
			if (!OffsetContainedInComponentArray(NewComponents, ObjectOffset))
			{
				if (UObject* Object = NetDriver->PackageMap->GetObjectFromNetGUID(OffsetToNetGuidIterator->Value, false))
				{
					UE_LOG(LogClientNetLoadActorHelper, Verbose,
						   TEXT("A SubObject (ObjectRef offset: %u) on bNetLoadOnClient actor with entityId %s was destroyed while the "
								"actor was out of the client's interest. Destroying the SubObject now."),
						   ObjectOffset, *EntityId.ToString());
					const FUnrealObjectRef ObjectRef(EntityId, ObjectOffset);
					NetDriver->ActorSystem.Get()->DestroySubObject(EntityId, *Object, ObjectRef);
				}
				OffsetToNetGuidIterator.RemoveCurrent();
			}
		}
	}
}

bool FClientNetLoadActorHelper::OffsetContainedInComponentArray(const TArray<ComponentData>& Components,
																const ObjectOffset OffsetToCheckIfContained) const
{
	for (const ComponentData& Component : Components)
	{
		// Skip if this isn't a generated component
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
};

} // namespace SpatialGDK
