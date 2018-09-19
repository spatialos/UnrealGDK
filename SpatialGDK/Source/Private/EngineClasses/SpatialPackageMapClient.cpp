// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialPackageMapClient.h"

#include "GameFramework/Actor.h"
#include "EngineUtils.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialSender.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialConstants.h"
#include "Utils/EntityRegistry.h"

DEFINE_LOG_CATEGORY(LogSpatialOSPackageMap);

void GetSubobjects(UObject* Object, TArray<UObject*>& InSubobjects)
{
	InSubobjects.Empty();
	ForEachObjectWithOuter(Object, [&InSubobjects](UObject* Object)
	{
		// Objects can only be allocated NetGUIDs if this is true.
		if (Object->IsSupportedForNetworking() && !Object->IsPendingKill() && !Object->IsEditorOnly())
		{
			// Walk up the outer chain and ensure that no object is PendingKill. This is required because although
			// EInternalObjectFlags::PendingKill prevents objects that are PendingKill themselves from getting added
			// to the list, it'll still add children of PendingKill objects. This then causes an assertion within 
			// FNetGUIDCache::RegisterNetGUID_Server where it again iterates up the object's owner chain, assigning
			// ids and ensuring that no object is set to PendingKill in the process.
			UObject* Outer = Object->GetOuter();
			while (Outer != nullptr)
			{
				if (Outer->IsPendingKill())
				{
					return;
				}
				Outer = Outer->GetOuter();
			}
			InSubobjects.Add(Object);
		}
	}, true, RF_NoFlags, EInternalObjectFlags::PendingKill);

	InSubobjects.StableSort([](UObject& A, UObject& B)
	{
		return A.GetName() < B.GetName();
	});
}

FNetworkGUID USpatialPackageMapClient::ResolveEntityActor(AActor* Actor, Worker_EntityId EntityId, const SubobjectToOffsetMap& SubobjectToOffset)
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	FNetworkGUID NetGUID = SpatialGuidCache->GetNetGUIDFromEntityId(EntityId);

	// check we haven't already assigned a NetGUID to this object
	if (!NetGUID.IsValid())
	{
		NetGUID = SpatialGuidCache->AssignNewEntityActorNetGUID(Actor, SubobjectToOffset);
	}
	return NetGUID;
}

void USpatialPackageMapClient::RemoveEntityActor(Worker_EntityId EntityId)
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());

	if (SpatialGuidCache->GetNetGUIDFromEntityId(EntityId).IsValid())
	{
		SpatialGuidCache->RemoveEntityNetGUID(EntityId);
	}
}

void USpatialPackageMapClient::RemoveEntitySubobjects(Worker_EntityId EntityId)
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	if (SpatialGuidCache->GetNetGUIDFromEntityId(EntityId).IsValid())
	{
		SpatialGuidCache->RemoveEntitySubobjectsNetGUIDs(EntityId);
	}
}

FNetworkGUID USpatialPackageMapClient::ResolveStablyNamedObject(const UObject* Object)
{
	check(Object->IsFullNameStableForNetworking());
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	return SpatialGuidCache->AssignNewStablyNamedObjectNetGUID(Object);
}

UnrealObjectRef USpatialPackageMapClient::GetUnrealObjectRefFromNetGUID(const FNetworkGUID & NetGUID) const
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	return SpatialGuidCache->GetUnrealObjectRefFromNetGUID(NetGUID);
}

FNetworkGUID USpatialPackageMapClient::GetNetGUIDFromUnrealObjectRef(const UnrealObjectRef & ObjectRef) const
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	return SpatialGuidCache->GetNetGUIDFromUnrealObjectRef(ObjectRef);
}

// Rebase - Old code and needs merging with above.
FNetworkGUID FSpatialNetGUIDCache::GetNetGUIDFromUnrealObjectRef(const UnrealObjectRef& ObjectRef)
{
	UnrealObjectRef NetRemappedObjectRef = ObjectRef;
	NetworkRemapObjectRefPaths(NetRemappedObjectRef);
	return GetNetGUIDFromUnrealObjectRefInternal(NetRemappedObjectRef);
}

FNetworkGUID FSpatialNetGUIDCache::GetNetGUIDFromUnrealObjectRefInternal(const UnrealObjectRef& ObjectRef)
{
	FNetworkGUID* CachedGUID = UnrealObjectRefToNetGUID.Find(ObjectRef);
	FNetworkGUID NetGUID = CachedGUID ? *CachedGUID : FNetworkGUID{};
	if (!NetGUID.IsValid() && ObjectRef.Path.IsSet())
	{
		FNetworkGUID OuterGUID;
		if (ObjectRef.Outer.IsSet())
		{
			OuterGUID = GetNetGUIDFromUnrealObjectRef(ObjectRef.Outer.GetValue());
		}
		NetGUID = RegisterNetGUIDFromPath(ObjectRef.Path.GetValue(), OuterGUID);
		RegisterObjectRef(NetGUID, ObjectRef);
	}
	return NetGUID;
}

FNetworkGUID USpatialPackageMapClient::GetNetGUIDFromEntityId(const Worker_EntityId& EntityId) const
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	UnrealObjectRef ObjectRef(EntityId, 0);
	return GetNetGUIDFromUnrealObjectRef(ObjectRef);
}

bool USpatialPackageMapClient::SerializeObject(FArchive& Ar, UClass* InClass, UObject*& Obj, FNetworkGUID *OutNetGUID)
{
	// Super::SerializeObject is not called here on purpose
	Ar << Obj;

	return true;
}

FSpatialNetGUIDCache::FSpatialNetGUIDCache(USpatialNetDriver* InDriver)
	: FNetGUIDCache(InDriver)
{
}

FNetworkGUID FSpatialNetGUIDCache::AssignNewEntityActorNetGUID(AActor* Actor, const SubobjectToOffsetMap& SubobjectToOffset)
{
	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(Driver);

	Worker_EntityId EntityId = SpatialNetDriver->GetEntityRegistry()->GetEntityIdFromActor(Actor);
	check(EntityId > 0);

	// Get interop.
	USpatialReceiver* Receiver = SpatialNetDriver->Receiver;

	// Set up the NetGUID and ObjectRef for this actor.
	FNetworkGUID NetGUID = GetOrAssignNetGUID_SpatialGDK(Actor);
	UnrealObjectRef ObjectRef(EntityId, 0);
	RegisterObjectRef(NetGUID, ObjectRef);
	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Registered new object ref for actor: %s. NetGUID: %s, entity ID: %lld"),
		*Actor->GetName(), *NetGUID.ToString(), EntityId);

	Receiver->ResolvePendingOperations(Actor, ObjectRef);

	// Allocate NetGUIDs for each subobject, sorting alphabetically to ensure stable references.
	TArray<UObject*> ActorSubobjects;
	GetSubobjects(Actor, ActorSubobjects);

	for (UObject* Subobject : ActorSubobjects)
	{
		const uint32* Offset = SubobjectToOffset.Find(*Subobject->GetName());
		if (Offset != nullptr)
		{
			FNetworkGUID SubobjectNetGUID = GetOrAssignNetGUID_SpatialGDK(Subobject);
			UnrealObjectRef SubobjectRef(EntityId, *Offset);
			RegisterObjectRef(SubobjectNetGUID, SubobjectRef);
			UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Registered new object ref for subobject %s inside actor %s. NetGUID: %s, object ref: %s"),
				*Subobject->GetName(), *Actor->GetName(), *SubobjectNetGUID.ToString(), *SubobjectRef.ToString());
			Receiver->ResolvePendingOperations(Subobject, SubobjectRef);
		}
	}

	return NetGUID;
}

// Recursively assign netguids to the outer chain of a UObject. Then associate them with their Spatial representation (UnrealObjectRef)
// This is required in order to be able to refer to a non-replicated stably named UObject.
// Dynamically spawned actors and references to their subobjects do not go through this codepath.
FNetworkGUID FSpatialNetGUIDCache::AssignNewStablyNamedObjectNetGUID(const UObject* Object)
{
	FNetworkGUID NetGUID = GetOrAssignNetGUID_SpatialGDK(Object);
	FNetworkGUID OuterGUID;
	UObject* OuterObject = Object->GetOuter();

	if (OuterObject)
	{
		OuterGUID = AssignNewStablyNamedObjectNetGUID(OuterObject);
	}

	UnrealObjectRef StablyNamedObjRef(0, 0, Object->GetFName().ToString(), (OuterGUID.IsValid() && !OuterGUID.IsDefault()) ? GetUnrealObjectRefFromNetGUID(OuterGUID) : UnrealObjectRef());
	RegisterObjectRef(NetGUID, StablyNamedObjRef);

	return NetGUID;
}

void FSpatialNetGUIDCache::RemoveEntityNetGUID(Worker_EntityId EntityId)
{
	FNetworkGUID EntityNetGUID = GetNetGUIDFromEntityId(EntityId);
	UnrealObjectRef* ActorRef = NetGUIDToUnrealObjectRef.Find(EntityNetGUID);
	NetGUIDToUnrealObjectRef.Remove(EntityNetGUID);
	UnrealObjectRefToNetGUID.Remove(*ActorRef);
}

void FSpatialNetGUIDCache::RemoveEntitySubobjectsNetGUIDs(Worker_EntityId EntityId)
{
	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(Driver);
	SubobjectToOffsetMap SubobjectNameToOffset = SpatialNetDriver->View->GetUnrealMetadata(EntityId)->SubobjectNameToOffset;

	for (const auto& Pair : SubobjectNameToOffset)
	{
		UnrealObjectRef SubobjectRef(EntityId, Pair.Value);
		if (FNetworkGUID* SubobjectNetGUID = UnrealObjectRefToNetGUID.Find(SubobjectRef))
		{
			NetGUIDToUnrealObjectRef.Remove(*SubobjectNetGUID);
			UnrealObjectRefToNetGUID.Remove(SubobjectRef);
		}
	}
}

FNetworkGUID FSpatialNetGUIDCache::GetNetGUIDFromUnrealObjectRef(const UnrealObjectRef& ObjectRef)
{
	FNetworkGUID* CachedGUID = UnrealObjectRefToNetGUID.Find(ObjectRef);
	FNetworkGUID NetGUID = CachedGUID ? *CachedGUID : FNetworkGUID{};
	if (!NetGUID.IsValid() && ObjectRef.Path.IsSet())
	{
		FNetworkGUID OuterGUID;
		if (ObjectRef.Outer.IsSet())
		{
			OuterGUID = GetNetGUIDFromUnrealObjectRef(ObjectRef.Outer.GetValue());
		}
		NetGUID = RegisterNetGUIDFromPath(ObjectRef.Path.GetValue(), OuterGUID);
		RegisterObjectRef(NetGUID, ObjectRef);
	}
	return NetGUID;
}

void FSpatialNetGUIDCache::NetworkRemapObjectRefPaths(UnrealObjectRef& ObjectRef) const
	// If we have paths, network-sanitize all of them (e.g. removing PIE prefix).
	if (ObjectRef.Path.IsSet())
	{
		UnrealObjectRef* Iterator = &ObjectRef;
		while (true) {
			if (!Iterator->Path.IsSet())
			{
				//UE_LOG(LogSpatialOSPackageMap, Warning, TEXT("%s: NetworkRemapObjectRefPaths found empty path"),
				//	*Cast<USpatialNetDriver>(Driver)->GetSpatialOS()->GetWorkerId());
			} 
			else
			{
				FString TempPath(*Iterator->Path);
				GEngine->NetworkRemapPath(Driver, TempPath, true);
				Iterator->Path = TempPath;
			}
			if (!Iterator->Outer.IsSet())
			{
				break;
			}
			Iterator = &Iterator->Outer.GetValue();
		}
	}
}

improbable::UnrealObjectRef FSpatialNetGUIDCache::GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const
{
	const UnrealObjectRef* ObjRef = NetGUIDToUnrealObjectRef.Find(NetGUID);
	return ObjRef ? (UnrealObjectRef)*ObjRef : SpatialConstants::UNRESOLVED_OBJECT_REF;
}

FNetworkGUID FSpatialNetGUIDCache::GetNetGUIDFromEntityId(Worker_EntityId EntityId) const
{
	UnrealObjectRef ObjRef(EntityId, 0);
	const FNetworkGUID* NetGUID = UnrealObjectRefToNetGUID.Find(ObjRef);
	return (NetGUID == nullptr ? FNetworkGUID(0) : *NetGUID);
}

FNetworkGUID FSpatialNetGUIDCache::RegisterNetGUIDFromPath(const FString& PathName, const FNetworkGUID& OuterGUID)
{
	// This function should only be called for stably named object references, not dynamic ones.
	FNetGuidCacheObject CacheObject;
	CacheObject.PathName = FName(*PathName);
	CacheObject.OuterGUID = OuterGUID;
	FNetworkGUID NetGUID = GenerateNewNetGUID(0);
	RegisterNetGUID_Internal(NetGUID, CacheObject);
	return NetGUID;
}

FNetworkGUID FSpatialNetGUIDCache::GenerateNewNetGUID(const int32 IsStatic)
{
	// Here we have to borrow from FNetGuidCache::AssignNewNetGUID_Server to avoid a source change.
#define COMPOSE_NET_GUID(Index, IsStatic)	(((Index) << 1) | (IsStatic) )
#define ALLOC_NEW_NET_GUID(IsStatic)		(COMPOSE_NET_GUID(++UniqueNetIDs[IsStatic], IsStatic))

	// Generate new NetGUID and assign it
	FNetworkGUID NetGUID = FNetworkGUID(ALLOC_NEW_NET_GUID(IsStatic));
	return NetGUID;
}

FNetworkGUID FSpatialNetGUIDCache::GetOrAssignNetGUID_SpatialGDK(const UObject* Object)
{
	FNetworkGUID NetGUID = GetOrAssignNetGUID(Object);
	if (Object != nullptr)
	{
		//UE_LOG(LogSpatialOSPackageMap, Log, TEXT("%s: GetOrAssignNetGUID for object %s returned %s. IsDynamicObject: %d"),
		//	*Cast<USpatialNetDriver>(Driver)->GetSpatialOS()->GetWorkerId(),
		//	*Object->GetName(),
		//	*NetGUID.ToString(),
		//	(int)IsDynamicObject(Object));
	}

	// One major difference between how Unreal does NetGUIDs vs us is, we don't attempt to make them consistent across workers and client.
	// The function above might have returned without assigning new GUID, because we are the client.
	// Let's directly call the client function in that case.
	if (Object != nullptr && NetGUID == FNetworkGUID::GetDefault() && !IsNetGUIDAuthority())
	{
		NetGUID = GenerateNewNetGUID(IsDynamicObject(Object) ? 0 : 1);

		FNetGuidCacheObject CacheObject;
		CacheObject.Object = MakeWeakObjectPtr(const_cast<UObject*>(Object));
		CacheObject.PathName = Object->GetFName();
		CacheObject.OuterGUID = GetOrAssignNetGUID_SpatialGDK(Object->GetOuter());
		RegisterNetGUID_Internal(NetGUID, CacheObject);

		//UE_LOG(LogSpatialOSPackageMap, Log, TEXT("%s: NetGUID for object %s was not found in the cache. Generated new NetGUID %s."),
		//	*Cast<USpatialNetDriver>(Driver)->GetSpatialOS()->GetWorkerId(),
		//	*Object->GetName(),
		//	*NetGUID.ToString());
	}

	check((NetGUID.IsValid() && !NetGUID.IsDefault()) || Object == nullptr);
	return NetGUID;
}

void FSpatialNetGUIDCache::RegisterObjectRef(FNetworkGUID NetGUID, const UnrealObjectRef& ObjectRef)
{
	checkSlow(!NetGUIDToUnrealObjectRef.Contains(NetGUID) || (NetGUIDToUnrealObjectRef.Contains(NetGUID) && NetGUIDToUnrealObjectRef.FindChecked(NetGUID) == ObjectRef));
	checkSlow(!UnrealObjectRefToNetGUID.Contains(ObjectRef) || (UnrealObjectRefToNetGUID.Contains(ObjectRef) && UnrealObjectRefToNetGUID.FindChecked(ObjectRef) == NetGUID));
	NetGUIDToUnrealObjectRef.Emplace(NetGUID, ObjectRef);
	UnrealObjectRefToNetGUID.Emplace(ObjectRef, NetGUID);
}
