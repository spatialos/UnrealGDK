// Fill out your copyright notice in the Description page of Project Settings.

#include "SpatialPackageMapClient.h"
#include "EntityRegistry.h"
#include "SpatialNetDriver.h"



struct FExportFlags
{
	union
	{
		struct
		{
			uint8 bHasPath : 1;
			uint8 bNoLoad : 1;
			uint8 bHasNetworkChecksum : 1;
		};

		uint8	Value;
	};

	FExportFlags()
	{
		Value = 0;
	}
};


FSpatialNetGUIDCache::FSpatialNetGUIDCache(USpatialNetDriver* InDriver)
	: FNetGUIDCache(InDriver)
{

}


FNetworkGUID FSpatialNetGUIDCache::AssignNewNetGUID_Server(const UObject* Object)
{
	check(IsNetGUIDAuthority());

	const int32 IsStatic = IsDynamicObject(Object) ? 0 : 1;

	if (!IsStatic)
	{
		// Dynamic objects (excluding subobjects) should all be entities
		const AActor* Actor = Cast<AActor>(Object);	

		int64 Id = Cast<USpatialNetDriver>(Driver)->GetEntityRegistry()->GetEntityIdFromActor(Actor).ToSpatialEntityId();

		if (Id > 0)
		{	
			// Allocate a unique dynamic GUID (this just does what the ALLOC_NEW_NET_GUID macro does)
			FNetworkGUID GUID = (2 ^ 31) + ((++UniqueNetIDs[0]) << 1 | 0 );
			NetGuidToEntityIdMap.Emplace(GUID, Id);
			RegisterNetGUID_Server(GUID, Object);
			return GUID;
		}
	}

	return FNetGUIDCache::AssignNewNetGUID_Server(Object);
}

void FSpatialNetGUIDCache::RegisterSpatialNetGUID(const FNetworkGUID& NetGUID, const UObject* Object, const FString& Path)
{
	check(!ObjectLookup.Contains(NetGUID));
	FNetGuidCacheObject CacheObject;
	CacheObject.Object = Object;
	CacheObject.PathName = FName(*Path);
	RegisterNetGUID_Internal(NetGUID, CacheObject);
}

bool USpatialPackageMapClient::SerializeObject(FArchive& Ar, UClass* InClass, UObject*& Obj, FNetworkGUID *OutNetGUID /*= NULL*/)
{
	return Super::SerializeObject(Ar, InClass, Obj, OutNetGUID);
}

bool USpatialPackageMapClient::SerializeNewActor(FArchive& Ar, class UActorChannel *Channel, class AActor*& Actor)
{
	return Super::SerializeNewActor(Ar, Channel, Actor);
}

void USpatialPackageMapClient::RegisterStaticObjectGUID(FNetworkGUID& GUID, FString& Path)
{
	//FNetBitWriter Ar(sizeof(GUID) + sizeof(uint8) + sizeof(Path));

	//FExportFlags ExportFlags;
	//ExportFlags.bHasNetworkChecksum = 0;
	//ExportFlags.bHasPath = 1;

	//Ar << GUID;
	//Ar << ExportFlags.Value;
	//Ar << Path;

	//// pretend this is a received FArchive
	//Ar.ArIsSaving = 0;
	//Ar.ArIsLoading = 1;

	//FNetworkGUID TestGUID;
	//Ar << TestGUID; 
	//UE_LOG(LogTemp, Log, TEXT("Read GUID: %i"), TestGUID.Value);

	//FExportFlags TestFlags;
	//Ar << TestFlags.Value;
	//
	//FString TestPath;
	//Ar << TestPath;
	//UE_LOG(LogTemp, Log, TEXT("Read Path: %s"), *TestPath);

	//UObject* Obj = nullptr;
	//InternalLoadObject(Ar, Obj, 0);

	FStringAssetReference AssetRef(*Path);
	UObject* Object = AssetRef.TryLoad();
	static_cast<FSpatialNetGUIDCache*>(GuidCache.Get())->RegisterSpatialNetGUID(GUID, Object, Path);
}
