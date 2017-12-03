// Fill out your copyright notice in the Description page of Project Settings.

#include "SpatialPackageMapClient.h"
#include "EntityRegistry.h"
#include "SpatialNetDriver.h"

const uint32 StaticObjectOffset = 0x80000000; // 2^31

struct FCompareComponentNames
{
	bool operator()(UObject& A, UObject& B) const 
	{
		return A.GetName() < B.GetName();
	}
};

FSpatialNetGUIDCache::FSpatialNetGUIDCache(USpatialNetDriver* InDriver)
	: FNetGUIDCache(InDriver)
{

}


FNetworkGUID FSpatialNetGUIDCache::AssignNewNetGUID_Server(const UObject* Object)
{
	// Should only be assigning GUIDs for dynamic objects 
	// Static objects' NetGUIDs are predetermined and registered in RegisterPreallocatedNetGUID
	if (IsDynamicObject(Object))
	{
		return FNetGUIDCache::AssignNewNetGUID_Server(Object);
	}
	
	return FNetworkGUID(0);
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

	FNetworkGUID NetGUID = AssignNewNetGUID(Actor);
	check(NetGUID.IsValid());
	NetGUIDToEntityIdMap.Emplace(NetGUID, EntityId);

	// Allocate GUIDs for each subobject too
	TArray<UObject*> DefaultSubobjects;
	Actor->GetDefaultSubobjects(DefaultSubobjects);

	// Sorting alphabetically to ensure stable references
	Sort(DefaultSubobjects.GetData(), DefaultSubobjects.Num(), FCompareComponentNames());

	for (UObject* Subobject : DefaultSubobjects)
	{
		AssignNewNetGUID(Subobject);
	}

	return NetGUID;
}

FEntityId FSpatialNetGUIDCache::GetEntityIdFromNetGUID(const FNetworkGUID NetGUID)
{
	FEntityId* EntityId = NetGUIDToEntityIdMap.Find(NetGUID);
	return (EntityId == nullptr ? FEntityId(0) : *EntityId);
}

FNetworkGUID FSpatialNetGUIDCache::GetNetGUIDFromEntityId(const FEntityId EntityId)
{
	FNetworkGUID* NetGUID = EntityIdToNetGUIDMap.Find(EntityId);
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
	if (!SpatialGuidCache->GetNetGUIDFromEntityId(EntityId).IsValid())
	{ 
		SpatialGuidCache->AssignNewEntityActorNetGUID(Actor);
	}	
}
