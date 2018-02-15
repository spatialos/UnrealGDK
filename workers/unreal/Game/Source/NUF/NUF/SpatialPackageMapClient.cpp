// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialPackageMapClient.h"
#include "SpatialConstants.h"
#include "EntityRegistry.h"
#include "SpatialNetDriver.h"
#include "SpatialInterop.h"
#include "SpatialActorChannel.h"
#include "SpatialTypeBinding.h"

DEFINE_LOG_CATEGORY(LogSpatialOSPackageMap);

//const uint32 StaticObjectOffset = 0x80000000; // 2^31

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

	// Sort to ensure stable order.
	InSubobjects.StableSort([](UObject& A, UObject& B)
	{
		return A.GetName() < B.GetName();
	});
}

/*
void USpatialPackageMapClient::ResolveStaticObjectGUID(FNetworkGUID& NetGUID, FString& Path)
{
	// there should never be a case where a static object gets allocated in the dynamic region
	check(NetGUID.Value >= StaticObjectOffset);

	// check that we don't already have this object registered
	UObject* RegisteredObject = GuidCache->GetObjectFromNetGUID(NetGUID, false);
	if (RegisteredObject == nullptr)
	{
		FStringAssetReference AssetRef(*Path);
		UObject* Object = AssetRef.TryLoad();
		if (Object != nullptr)
		{
			FNetGuidCacheObject CacheObject;
			CacheObject.Object = Object;
			CacheObject.PathName = FName(*Path);
			static_cast<FSpatialNetGUIDCache*>(GuidCache.Get())->RegisterNetGUID_Internal(NetGUID, CacheObject);

		}
		else
		{
			UE_LOG(LogSpatialOSPackageMap, Warning, TEXT("Failed to resolve object with path: %s"), *Path);
		}
	}
}
*/

FNetworkGUID USpatialPackageMapClient::ResolveEntityActor(AActor* Actor, FEntityId EntityId)
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	FNetworkGUID NetGUID = SpatialGuidCache->GetNetGUIDFromEntityId(EntityId.ToSpatialEntityId());

	// check we haven't already assigned a NetGUID to this object
	if (!NetGUID.IsValid())
	{
		NetGUID = SpatialGuidCache->AssignNewEntityActorNetGUID(Actor);
	}
	return NetGUID;
}

bool USpatialPackageMapClient::SerializeNewActor(FArchive & Ar, UActorChannel * Channel, AActor *& Actor)
{
	bool bResult = Super::SerializeNewActor(Ar, Channel, Actor);
	//will remove the override if remains unused.
	return bResult;
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
	improbable::unreal::UnrealObjectRef ObjectRef{EntityId, 0};
	return GetNetGUIDFromUnrealObjectRef(ObjectRef);
}

void USpatialPackageMapClient::RegisterStaticObjects(const improbable::unreal::UnrealLevelData& LevelData)
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	SpatialGuidCache->RegisterStaticObjects(LevelData);
}

FSpatialNetGUIDCache::FSpatialNetGUIDCache(USpatialNetDriver* InDriver)
	: FNetGUIDCache(InDriver)
{
}

FNetworkGUID FSpatialNetGUIDCache::AssignNewEntityActorNetGUID(AActor* Actor)
{
	FEntityId EntityId = Cast<USpatialNetDriver>(Driver)->GetEntityRegistry()->GetEntityIdFromActor(Actor);
	check(EntityId.ToSpatialEntityId() > 0);

	// Get interop.
	USpatialInterop* Interop = Cast<USpatialNetDriver>(Driver)->GetSpatialInterop();
	check(Interop);

	// Set up the NetGUID and ObjectRef for this actor.
	FNetworkGUID NetGUID = GetOrAssignNetGUID_NUF(Actor);
	improbable::unreal::UnrealObjectRef ObjectRef{EntityId.ToSpatialEntityId(), 0};
	RegisterObjectRef(NetGUID, ObjectRef);
	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Registered new object ref for actor: %s. NetGUID: %s, entity ID: %d"),
		*Actor->GetName(), *NetGUID.ToString(), EntityId.ToSpatialEntityId());
	Interop->ResolveObject(Actor, ObjectRef);

	// Allocate NetGUIDs for each subobject, sorting alphabetically to ensure stable references.
	TArray<UObject*> ActorSubobjects;
	GetSubobjects(Actor, ActorSubobjects);
	uint32 SubobjectOffset = 0;
	for (UObject* Subobject : ActorSubobjects)
	{
		SubobjectOffset++;
		FNetworkGUID SubobjectNetGUID = GetOrAssignNetGUID_NUF(Subobject);
		improbable::unreal::UnrealObjectRef SubobjectRef{EntityId.ToSpatialEntityId(), SubobjectOffset};
		RegisterObjectRef(SubobjectNetGUID, SubobjectRef);
		UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Registered new object ref for subobject %s inside actor %s. NetGUID: %s, object ref: (entity ID: %d, offset: %d)"),
			*Subobject->GetName(), *Actor->GetName(), *SubobjectNetGUID.ToString(), EntityId.ToSpatialEntityId(), SubobjectOffset);
		Interop->ResolveObject(Subobject, SubobjectRef);
	}

	return NetGUID;
}

FNetworkGUID FSpatialNetGUIDCache::GetNetGUIDFromUnrealObjectRef(const improbable::unreal::UnrealObjectRef& ObjectRef) const
{
	const FNetworkGUID* NetGUID = UnrealObjectRefToNetGUID.Find(ObjectRef);
	return (NetGUID == nullptr ? FNetworkGUID(0) : *NetGUID);
}

improbable::unreal::UnrealObjectRef FSpatialNetGUIDCache::GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const
{
	const FHashableUnrealObjectRef* ObjRef = NetGUIDToUnrealObjectRef.Find(NetGUID);
	return ObjRef ? *ObjRef : SpatialConstants::UNRESOLVED_OBJECT_REF;
}

FNetworkGUID FSpatialNetGUIDCache::GetNetGUIDFromEntityId(const worker::EntityId& EntityId) const
{
	improbable::unreal::UnrealObjectRef ObjRef{EntityId, 0};
	const FNetworkGUID* NetGUID = UnrealObjectRefToNetGUID.Find(ObjRef);
	return (NetGUID == nullptr ? FNetworkGUID(0) : *NetGUID);
}

void FSpatialNetGUIDCache::RegisterStaticObjects(const improbable::unreal::UnrealLevelData& LevelData)
{
	// Get interop.
	USpatialInterop* Interop = Cast<USpatialNetDriver>(Driver)->GetSpatialInterop();
	check(Interop);

	// Build list of static objects in the world.
	UWorld* World = Driver->GetWorld();
	TMap<FString, AActor*> StaticActorsInWorld;
	for (TActorIterator<AActor> Itr(World); Itr; ++Itr)
	{
		AActor* Actor = *Itr;
		FString PathName = Actor->GetPathName(World);
		StaticActorsInWorld.Add(PathName, Actor);
		UE_LOG(LogSpatialOSPackageMap, Log, TEXT("%s: Static object in world: %s."), *Interop->GetSpatialOS()->GetWorkerId(), *PathName);
	}

	// Match the above list with the static actor data.
	auto& LevelDataActors = LevelData.static_actor_map();
	for (auto& Pair : LevelDataActors)
	{
		const char* LevelDataActorPath = Pair.second.c_str();
		AActor* Actor = StaticActorsInWorld.FindRef(UTF8_TO_TCHAR(LevelDataActorPath));

		// Skip objects which don't exist.
		if (!Actor)
		{
			UE_LOG(LogSpatialOSPackageMap, Warning, TEXT("Expected object not found in level: %s"), UTF8_TO_TCHAR(LevelDataActorPath));
			continue;
		}

		// Skip objects which we've already registered.
		if (NetGUIDLookup.FindRef(Actor).IsValid())
		{
			continue;
		}

		// Register static NetGUID.
		FNetworkGUID NetGUID = AssignStaticActorNetGUID(Actor, FNetworkGUID(Pair.first));
		Interop->ResolveObject(Actor, GetUnrealObjectRefFromNetGUID(NetGUID));

		// Deal with sub-objects of static objects.
		// TODO(David): Ensure that the NetGUID allocated in the snapshot generator (which are always > 0x7fffffff) ensures that there's enough space
		// for sub-objects to be stored in increasing NetGUIDs after the static object one (currently, they're just hashes of the path).
		TArray<UObject*> StaticSubobjects;
		GetSubobjects(Actor, StaticSubobjects);
		uint32 SubobjectOffset = 0;
		for (UObject* Subobject : StaticSubobjects)
		{
			SubobjectOffset++;
			FNetworkGUID SubobjectNetGUID = AssignStaticActorNetGUID(Subobject, FNetworkGUID(Pair.first + SubobjectOffset));
			Interop->ResolveObject(Subobject, GetUnrealObjectRefFromNetGUID(SubobjectNetGUID));
		}
	}
}

// TODO(David): Do something with this function.
/*
FNetworkGUID FSpatialNetGUIDCache::AssignNewNetGUID(const UObject* Object)
{
	FNetworkGUID GUID = (0x80000000 - 1) & ++UniqueNetIDs[0];
	FNetGuidCacheObject CacheObject;
	CacheObject.Object = Object;

	RegisterNetGUID_Internal(GUID, CacheObject);
	return GUID;
}
*/

FNetworkGUID FSpatialNetGUIDCache::GetOrAssignNetGUID_NUF(const UObject* Object)
{
	FNetworkGUID NetGUID = GetOrAssignNetGUID(Object);
	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("%s: GetOrAssignNetGUID for object %s returned %s. IsDynamicObject: %d"),
		*Cast<USpatialNetDriver>(Driver)->GetSpatialOS()->GetWorkerId(),
		*Object->GetName(),
		*NetGUID.ToString(),
		(int)IsDynamicObject(Object));

	// One major difference between how Unreal does NetGUIDs vs us is, we don't attempt to make them consistent across workers and client.
	// The function above might have returned without assigning new GUID, because we are the client.
	// Let's directly call the client function in that case.
	if (NetGUID == FNetworkGUID::GetDefault() && !IsNetGUIDAuthority() && IsDynamicObject(Object)) //todo-giray: Support static objects
	{
		// Here we have to borrow from FNetGuidCache::AssignNewNetGUID_Server to avoid a source change.
#define COMPOSE_NET_GUID(Index, IsStatic)	(((Index) << 1) | (IsStatic) )
#define ALLOC_NEW_NET_GUID(IsStatic)		(COMPOSE_NET_GUID(++UniqueNetIDs[IsStatic], IsStatic))

		// Generate new NetGUID and assign it
		const int32 IsStatic = IsDynamicObject(Object) ? 0 : 1;

		NetGUID = FNetworkGUID(ALLOC_NEW_NET_GUID(IsStatic));
		RegisterNetGUID_Client(NetGUID, Object);

		UE_LOG(LogSpatialOSPackageMap, Log, TEXT("%s: NetGUID for object %s was not found in the cache. Generated new NetGUID %s."),
			*Cast<USpatialNetDriver>(Driver)->GetSpatialOS()->GetWorkerId(),
			*Object->GetName(),
			*NetGUID.ToString());
	}

	check(NetGUID.IsValid());
	check(!NetGUID.IsDefault());
	return NetGUID;
}

void FSpatialNetGUIDCache::RegisterObjectRef(FNetworkGUID NetGUID, const improbable::unreal::UnrealObjectRef& ObjectRef)
{
	checkSlow(!NetGUIDToUnrealObjectRef.Contains(NetGUID) || (NetGUIDToUnrealObjectRef.Contains(NetGUID) && NetGUIDToUnrealObjectRef.FindChecked(NetGUID) == ObjectRef));
	checkSlow(!UnrealObjectRefToNetGUID.Contains(ObjectRef) || (UnrealObjectRefToNetGUID.Contains(ObjectRef) && UnrealObjectRefToNetGUID.FindChecked(ObjectRef) == NetGUID));
	NetGUIDToUnrealObjectRef.Emplace(NetGUID, ObjectRef);
	UnrealObjectRefToNetGUID.Emplace(ObjectRef, NetGUID);
}

FNetworkGUID FSpatialNetGUIDCache::AssignStaticActorNetGUID(const UObject* Object, const FNetworkGUID& StaticNetGUID)
{
	check(!NetGUIDLookup.FindRef(Object).IsValid());

	FNetGuidCacheObject CacheObject;
	CacheObject.Object = Object;
	CacheObject.PathName = Object->GetFName();
	RegisterNetGUID_Internal(StaticNetGUID, CacheObject);

	// Register object ref.
	improbable::unreal::UnrealObjectRef ObjectRef{0, StaticNetGUID.Value};
	RegisterObjectRef(StaticNetGUID, ObjectRef);

	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("%s: Registered static object %s. NetGUID %s (entity ID: %llu, offset: %u)."),
		*Cast<USpatialNetDriver>(Driver)->GetSpatialOS()->GetWorkerId(),
		*Object->GetName(),
		*StaticNetGUID.ToString(),
		ObjectRef.entity(),
		ObjectRef.offset());

	return StaticNetGUID;
}
