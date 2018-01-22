// Fill out your copyright notice in the Description page of Project Settings.

#include "SpatialPackageMapClient.h"
#include "EntityRegistry.h"
#include "SpatialNetDriver.h"
#include "SpatialActorChannel.h"

const uint32 StaticObjectOffset = 0x80000000; // 2^31

struct FCompareComponentNames
{
	bool operator()(UObject& A, UObject& B) const 
	{
		return A.GetName() < B.GetName();
	}
};


improbable::unreal::UnrealObjectRef USpatialPackageMapClient::GetUnrealObjectRefFromNetGUID(const FNetworkGUID & NetGUID) const
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	check(SpatialGuidCache);
	return SpatialGuidCache->GetUnrealObjectRefFromNetGUID(NetGUID);
}

FNetworkGUID USpatialPackageMapClient::GetNetGUIDFromUnrealObjectRef(const improbable::unreal::UnrealObjectRef & ObjectRef) const
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	check(SpatialGuidCache);
	return SpatialGuidCache->GetNetGUIDFromUnrealObjectRef(ObjectRef);
}

FNetworkGUID USpatialPackageMapClient::GetNetGUIDFromEntityId(const worker::EntityId & EntityId) const
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	check(SpatialGuidCache);
	improbable::unreal::UnrealObjectRef ObjectRef{ EntityId, 0 };
	return GetNetGUIDFromUnrealObjectRef(ObjectRef);
}

FSpatialNetGUIDCache::FSpatialNetGUIDCache(USpatialNetDriver* InDriver)
	: FNetGUIDCache(InDriver)
{

}

FNetworkGUID FSpatialNetGUIDCache::AssignNewNetGUID_Server(const UObject* Object)
{
	//todo-giray: Making this return early for now as it causes SerializeNewActor to fail on clients.

	return FNetGUIDCache::AssignNewNetGUID_Server(Object);
	/*

	// Should only be assigning GUIDs for dynamic objects 
	// Static objects' NetGUIDs are predetermined and registered in RegisterPreallocatedNetGUID
	if (IsDynamicObject(Object))
	{
	return FNetGUIDCache::AssignNewNetGUID_Server(Object);
	}
	
	return FNetworkGUID(0);*/
}

FNetworkGUID FSpatialNetGUIDCache::AssignNewNetGUID(const UObject* Object)
{
	
	FNetworkGUID GUID = (0x80000000 - 1) & ++UniqueNetIDs[0];
	FNetGuidCacheObject CacheObject;
	CacheObject.Object = Object;

	RegisterNetGUID_Internal(GUID, CacheObject);
	return GUID;
}

FNetworkGUID FSpatialNetGUIDCache::AssignNewEntityActorNetGUID(AActor* Actor)
{
	FEntityId EntityId = Cast<USpatialNetDriver>(Driver)->GetEntityRegistry()->GetEntityIdFromActor(Actor);
	check(EntityId.ToSpatialEntityId() >= 0)

	FNetworkGUID NetGUID = GetOrAssignNetGUID(Actor);
	//One major difference between how Unreal does NetGUIDs vs us is, we don't attempt to make them consistent across workers and client.
	// The function above might have returned without assigning new GUID, because we are the client.
	// Let's directly call the client function in that case.
	if (NetGUID == FNetworkGUID::GetDefault() && !IsNetGUIDAuthority() && IsDynamicObject(Actor)) //todo-giray: Support static objects
	{
		// Here we have to borrow from FNetGuidCache::AssignNewNetGUID_Server to avoid source change
#define COMPOSE_NET_GUID( Index, IsStatic )	( ( ( Index ) << 1 ) | ( IsStatic ) )
#define ALLOC_NEW_NET_GUID( IsStatic )		( COMPOSE_NET_GUID( ++UniqueNetIDs[ IsStatic ], IsStatic ) )

		// Generate new NetGUID and assign it
		const int32 IsStatic = IsDynamicObject(Actor) ? 0 : 1;

		const FNetworkGUID NewNetGuid(ALLOC_NEW_NET_GUID(IsStatic));
		RegisterNetGUID_Client(NewNetGuid, Actor);
	}

	check(NetGUID.IsValid());
	uint32 SubObjOffset = 0;
	improbable::unreal::UnrealObjectRef ObjRef{ EntityId.ToSpatialEntityId(), SubObjOffset };
	UnrealObjectRefWrapper ObjRefWrapper;
	ObjRefWrapper.ObjectRef = ObjRef;
	NetGUIDToUnrealObjectRef.Emplace(NetGUID, ObjRefWrapper);
	UnrealObjectRefToNetGUID.Emplace(ObjRefWrapper, NetGUID);

	// Allocate GUIDs for each subobject too
	TArray<UObject*> DefaultSubobjects;
	Actor->GetDefaultSubobjects(DefaultSubobjects);

	// Sorting alphabetically to ensure stable references
	Sort(DefaultSubobjects.GetData(), DefaultSubobjects.Num(), FCompareComponentNames());

	for (UObject* Subobject : DefaultSubobjects)
	{
		SubObjOffset++;
		FNetworkGUID SubObjectNetGUID = GetOrAssignNetGUID(Subobject);
		ObjRefWrapper.ObjectRef.set_offset(SubObjOffset);
		NetGUIDToUnrealObjectRef.Emplace(SubObjectNetGUID, ObjRefWrapper);
		UnrealObjectRefToNetGUID.Emplace(ObjRefWrapper, SubObjectNetGUID);
		check(SubObjectNetGUID.IsValid());
	}

	return NetGUID;
}

improbable::unreal::UnrealObjectRef FSpatialNetGUIDCache::GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const
{
	const UnrealObjectRefWrapper* ObjRefWrapper = NetGUIDToUnrealObjectRef.Find(NetGUID);

	// If not found, return entity id 0 as it's not a valid entity id.
	return ObjRefWrapper ? ObjRefWrapper->ObjectRef : improbable::unreal::UnrealObjectRef{ 0, 0 };
}

FNetworkGUID FSpatialNetGUIDCache::GetNetGUIDFromUnrealObjectRef(const improbable::unreal::UnrealObjectRef& ObjectRef) const
{
	UnrealObjectRefWrapper ObjRefWrapper;
	ObjRefWrapper.ObjectRef = ObjectRef;
	const FNetworkGUID* NetGUID = UnrealObjectRefToNetGUID.Find(ObjRefWrapper);
	return (NetGUID == nullptr ? FNetworkGUID(0) : *NetGUID);
}

FNetworkGUID FSpatialNetGUIDCache::GetNetGUIDFromEntityId(const worker::EntityId& EntityId) const
{
	UnrealObjectRefWrapper ObjRefWrapper;
	improbable::unreal::UnrealObjectRef ObjRef{ EntityId, 0 };
	ObjRefWrapper.ObjectRef = ObjRef;
	const FNetworkGUID* NetGUID = UnrealObjectRefToNetGUID.Find(ObjRefWrapper);
	return (NetGUID == nullptr ? FNetworkGUID(0) : *NetGUID);
}

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
			UE_LOG(LogTemp, Warning, TEXT("Failed to resolve object with path: %s"), *Path);
		}
	}
}

void USpatialPackageMapClient::ResolveEntityActor(AActor* Actor, FEntityId EntityId)
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());

	// check we haven't already assigned a NetGUID to this object
	if (!SpatialGuidCache->GetNetGUIDFromEntityId(EntityId.ToSpatialEntityId()).IsValid())
	{
		SpatialGuidCache->AssignNewEntityActorNetGUID(Actor);
	}
}

bool USpatialPackageMapClient::SerializeNewActor(FArchive & Ar, UActorChannel * Channel, AActor *& Actor)
{
	bool bResult = Super::SerializeNewActor(Ar, Channel, Actor);
	//will remove the override if remains unused.
	return bResult;
}
