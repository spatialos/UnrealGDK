// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialPackageMapClient.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"
#include "EntityRegistry.h"
#include "SpatialActorChannel.h"
#include "SpatialConstants.h"
#include "SpatialInterop.h"
#include "SpatialNetDriver.h"
#include "SpatialTypeBinding.h"

DEFINE_LOG_CATEGORY(LogSpatialOSPackageMap);

void GetSubobjects(UObject* Object, TArray<UObject*>& InSubobjects)
{
	InSubobjects.Empty();
	ForEachObjectWithOuter(Object, [&InSubobjects](UObject* Object)
	{
		// Objects can only be allocated NetGUIDs if this is true.
		if (Object->IsSupportedForNetworking() && !Object->IsPendingKill() && !Object->IsEditorOnly())
		{
			InSubobjects.Add(Object);
		}
	});

	InSubobjects.StableSort([](UObject& A, UObject& B)
	{
		return A.GetName() < B.GetName();
	});
}

FNetworkGUID USpatialPackageMapClient::ResolveEntityActor(AActor* Actor, FEntityId EntityId, const SubobjectToOffsetMap& SubobjectToOffset)
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	FNetworkGUID NetGUID = SpatialGuidCache->GetNetGUIDFromEntityId(EntityId.ToSpatialEntityId());

	// check we haven't already assigned a NetGUID to this object
	if (!NetGUID.IsValid())
	{
		NetGUID = SpatialGuidCache->AssignNewEntityActorNetGUID(Actor, SubobjectToOffset);
	}
	return NetGUID;
}

void USpatialPackageMapClient::RemoveEntityActor(const FEntityId& EntityId)
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	if (SpatialGuidCache->GetNetGUIDFromEntityId(EntityId.ToSpatialEntityId()).IsValid())
	{
		SpatialGuidCache->RemoveEntityNetGUID(EntityId.ToSpatialEntityId());
	}
}

FNetworkGUID USpatialPackageMapClient::ResolveStablyNamedObject(const UObject* Object)
{
	check(Object->IsFullNameStableForNetworking());
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	return SpatialGuidCache->AssignNewStablyNamedObjectNetGUID(Object);	
}

improbable::unreal::UnrealObjectRef USpatialPackageMapClient::GetUnrealObjectRefFromNetGUID(const FNetworkGUID & NetGUID) const
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	return SpatialGuidCache->GetUnrealObjectRefFromNetGUID(NetGUID);
}

FNetworkGUID USpatialPackageMapClient::GetNetGUIDFromUnrealObjectRef(const improbable::unreal::UnrealObjectRef & ObjectRef) const
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	return SpatialGuidCache->GetNetGUIDFromUnrealObjectRef(ObjectRef);
}

FNetworkGUID USpatialPackageMapClient::GetNetGUIDFromEntityId(const worker::EntityId & EntityId) const
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	improbable::unreal::UnrealObjectRef ObjectRef{ EntityId, 0, worker::Option<std::string>{}, worker::Option<improbable::unreal::UnrealObjectRef>{} };
	return GetNetGUIDFromUnrealObjectRef(ObjectRef);
}

uint32 USpatialPackageMapClient::GetHashFromStaticClass(const UClass* StaticClass) const
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	return SpatialGuidCache->GetHashFromStaticClass(StaticClass);
}

UClass* USpatialPackageMapClient::GetStaticClassFromHash(uint32 Hash) const
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	return SpatialGuidCache->GetStaticClassFromHash(Hash);
}

FSpatialNetGUIDCache::FSpatialNetGUIDCache(USpatialNetDriver* InDriver)
	: FNetGUIDCache(InDriver)
{
	CreateStaticClassMapping();
}

FNetworkGUID FSpatialNetGUIDCache::AssignNewEntityActorNetGUID(AActor* Actor, const ::worker::Map< std::string, std::uint32_t >& SubobjectToOffset)
{
	FEntityId EntityId = Cast<USpatialNetDriver>(Driver)->GetEntityRegistry()->GetEntityIdFromActor(Actor);
	check(EntityId.ToSpatialEntityId() > 0);

	// Get interop.
	USpatialInterop* Interop = Cast<USpatialNetDriver>(Driver)->GetSpatialInterop();
	check(Interop);

	// Set up the NetGUID and ObjectRef for this actor.
	FNetworkGUID NetGUID = GetOrAssignNetGUID_SpatialGDK(Actor);
	improbable::unreal::UnrealObjectRef ObjectRef{ EntityId.ToSpatialEntityId(), 0, worker::Option<std::string>{}, worker::Option<improbable::unreal::UnrealObjectRef>{} };
	RegisterObjectRef(NetGUID, ObjectRef);
	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Registered new object ref for actor: %s. NetGUID: %s, entity ID: %lld"),
		*Actor->GetName(), *NetGUID.ToString(), EntityId.ToSpatialEntityId());
	Interop->ResolvePendingOperations(Actor, ObjectRef);

	// Allocate NetGUIDs for each subobject, sorting alphabetically to ensure stable references.
	TArray<UObject*> ActorSubobjects;
	GetSubobjects(Actor, ActorSubobjects);

	for (UObject* Subobject : ActorSubobjects)
	{
		auto OffsetIterator = SubobjectToOffset.find(std::string(TCHAR_TO_UTF8(*(Subobject->GetName()))));
		if (OffsetIterator != SubobjectToOffset.end()) {
			std::uint32_t Offset = OffsetIterator->second;

			FNetworkGUID SubobjectNetGUID = GetOrAssignNetGUID_SpatialGDK(Subobject);
			improbable::unreal::UnrealObjectRef SubobjectRef{EntityId.ToSpatialEntityId(), Offset, worker::Option<std::string>{}, worker::Option<improbable::unreal::UnrealObjectRef>{}};
			RegisterObjectRef(SubobjectNetGUID, SubobjectRef);
			UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Registered new object ref for subobject %s inside actor %s. NetGUID: %s, object ref: %s"),
				*Subobject->GetName(), *Actor->GetName(), *SubobjectNetGUID.ToString(), *ObjectRefToString(SubobjectRef));
			Interop->ResolvePendingOperations(Subobject, SubobjectRef);
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
	if (Object->GetOuter())
	{
		OuterGUID = AssignNewStablyNamedObjectNetGUID(Object->GetOuter());
	}
	improbable::unreal::UnrealObjectRef StablyNamedObjRef{ FEntityId{}.ToSpatialEntityId(),
		0,
		worker::Option<std::string>(TCHAR_TO_UTF8(*Object->GetFName().ToString())),
		(OuterGUID.IsValid() && !OuterGUID.IsDefault()) ? GetUnrealObjectRefFromNetGUID(OuterGUID) : worker::Option<improbable::unreal::UnrealObjectRef>{} };
	RegisterObjectRef(NetGUID, StablyNamedObjRef);

	return NetGUID;
}

void FSpatialNetGUIDCache::RemoveEntityNetGUID(worker::EntityId EntityId)
{
	FNetworkGUID EntityNetGUID = GetNetGUIDFromEntityId(EntityId);
	FHashableUnrealObjectRef* ActorRef = NetGUIDToUnrealObjectRef.Find(EntityNetGUID);
	NetGUIDToUnrealObjectRef.Remove(EntityNetGUID);
	UnrealObjectRefToNetGUID.Remove(*ActorRef);
}

void RemoveStablyNamedObjectNetGUID(const improbable::unreal::UnrealObjectRef& ObjectRef)
{
	// TODO
}

FNetworkGUID FSpatialNetGUIDCache::GetNetGUIDFromUnrealObjectRef(const improbable::unreal::UnrealObjectRef& ObjectRef)
{
	FNetworkGUID* CachedGUID = UnrealObjectRefToNetGUID.Find(ObjectRef);
	FNetworkGUID NetGUID = CachedGUID ? *CachedGUID : FNetworkGUID{};
	if (!NetGUID.IsValid() && !ObjectRef.path().empty())
	{
		FNetworkGUID OuterGUID;
		if (!ObjectRef.outer().empty())
		{
			OuterGUID = GetNetGUIDFromUnrealObjectRef(*ObjectRef.outer().data());
		}
		NetGUID = RegisterNetGUIDFromPath(FString(ObjectRef.path().data()->c_str()), OuterGUID);
		RegisterObjectRef(NetGUID, ObjectRef);
	}
	return NetGUID;
}

improbable::unreal::UnrealObjectRef FSpatialNetGUIDCache::GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const
{
	const FHashableUnrealObjectRef* ObjRef = NetGUIDToUnrealObjectRef.Find(NetGUID);
	return ObjRef ? (improbable::unreal::UnrealObjectRef)*ObjRef : SpatialConstants::UNRESOLVED_OBJECT_REF;
}

FNetworkGUID FSpatialNetGUIDCache::GetNetGUIDFromEntityId(worker::EntityId EntityId) const
{
	improbable::unreal::UnrealObjectRef ObjRef{EntityId, 0, worker::Option<std::string>{}, worker::Option<improbable::unreal::UnrealObjectRef>{}};
	const FNetworkGUID* NetGUID = UnrealObjectRefToNetGUID.Find(ObjRef);
	return (NetGUID == nullptr ? FNetworkGUID(0) : *NetGUID);
}

FNetworkGUID FSpatialNetGUIDCache::RegisterNetGUIDFromPath(const FString& PathName, const FNetworkGUID& OuterGUID)
{
	FNetGuidCacheObject CacheObject;
	CacheObject.PathName = FName(*PathName);
	CacheObject.OuterGUID = OuterGUID;
	FNetworkGUID NetGUID = GenerateNewNetGUID(0);
	RegisterNetGUID_Internal(NetGUID, CacheObject);
	return NetGUID;
}

uint32 FSpatialNetGUIDCache::GetHashFromStaticClass(const UClass* StaticClass) const
{
	return GetTypeHash(*StaticClass->GetPathName());
}

UClass* FSpatialNetGUIDCache::GetStaticClassFromHash(uint32 Hash) const
{
	// This should never fail in production code, but might in development if the client and server are running versions
	// with inconsistent static class lists.
	bool bContainsHash = StaticClassHashMap.Contains(Hash);
	checkf(bContainsHash, TEXT("Failed to find static class for hash: %d"), Hash);
	return bContainsHash ? StaticClassHashMap[Hash] : nullptr;
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
	if (Object)
	{
		UE_LOG(LogSpatialOSPackageMap, Log, TEXT("%s: GetOrAssignNetGUID for object %s returned %s. IsDynamicObject: %d"),
			*Cast<USpatialNetDriver>(Driver)->GetSpatialOS()->GetWorkerId(),
			*Object->GetName(),
			*NetGUID.ToString(),
			(int)IsDynamicObject(Object));
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

		UE_LOG(LogSpatialOSPackageMap, Log, TEXT("%s: NetGUID for object %s was not found in the cache. Generated new NetGUID %s."),
			*Cast<USpatialNetDriver>(Driver)->GetSpatialOS()->GetWorkerId(),
			*Object->GetName(),
			*NetGUID.ToString());
	}

	check((NetGUID.IsValid() && !NetGUID.IsDefault()) || Object == nullptr);
	return NetGUID;
}

void FSpatialNetGUIDCache::RegisterObjectRef(FNetworkGUID NetGUID, const improbable::unreal::UnrealObjectRef& ObjectRef)
{
	checkSlow(!NetGUIDToUnrealObjectRef.Contains(NetGUID) || (NetGUIDToUnrealObjectRef.Contains(NetGUID) && NetGUIDToUnrealObjectRef.FindChecked(NetGUID) == ObjectRef));
	checkSlow(!UnrealObjectRefToNetGUID.Contains(ObjectRef) || (UnrealObjectRefToNetGUID.Contains(ObjectRef) && UnrealObjectRefToNetGUID.FindChecked(ObjectRef) == NetGUID));
	NetGUIDToUnrealObjectRef.Emplace(NetGUID, ObjectRef);
	UnrealObjectRefToNetGUID.Emplace(ObjectRef, NetGUID);
}

void FSpatialNetGUIDCache::CreateStaticClassMapping()
{
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (!It->HasAnyClassFlags(CLASS_Abstract))
		{
			uint32 PathHash = GetHashFromStaticClass(*It);
			checkf(StaticClassHashMap.Contains(PathHash) == false, TEXT("Hash clash between %s and %s"), *It->GetPathName(), *StaticClassHashMap[PathHash]->GetPathName());
			StaticClassHashMap.Add(PathHash, *It);
		}
	}

	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Registered %d static classes to the class hashmap"), StaticClassHashMap.Num());
}
