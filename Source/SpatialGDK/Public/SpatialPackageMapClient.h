// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/PackageMapClient.h"
#include "EntityId.h"
#include "SpatialInterop.h"
#include "SpatialUnrealObjectRef.h"

#include <improbable/unreal/gdk/level_data.h>

#include "SpatialPackageMapClient.generated.h"

class USpatialActorChannel;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOSPackageMap, Log, All);

using SubobjectToOffsetMap = ::worker::Map< std::string, std::uint32_t >;

/**
 * 
 */
UCLASS()
class SPATIALGDK_API USpatialPackageMapClient : public UPackageMapClient
{
	GENERATED_BODY()		
public:
	FNetworkGUID ResolveEntityActor(AActor* Actor, FEntityId EntityId, const SubobjectToOffsetMap& SubobjectToOffset);
	void RemoveEntityActor(const FEntityId& EntityId);
	virtual bool SerializeNewActor(FArchive& Ar, class UActorChannel *Channel, class AActor*& Actor) override;

	improbable::unreal::gdk::UnrealObjectRef GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const;
	FNetworkGUID GetNetGUIDFromUnrealObjectRef(const improbable::unreal::gdk::UnrealObjectRef& ObjectRef) const;
	FNetworkGUID GetNetGUIDFromEntityId(const worker::EntityId& EntityId) const;

	void RegisterStaticObjects(const improbable::unreal::gdk::UnrealLevelData& LevelData);

	uint32 GetHashFromStaticClass(const UClass* StaticClass) const;
	UClass* GetStaticClassFromHash(uint32 Hash) const;

private:

};

class SPATIALGDK_API FSpatialNetGUIDCache : public FNetGUIDCache
{
public:
	FSpatialNetGUIDCache(class USpatialNetDriver* InDriver);
		
	FNetworkGUID AssignNewEntityActorNetGUID(AActor* Actor, const SubobjectToOffsetMap& SubobjectToOffset);
	void RemoveEntityNetGUID(worker::EntityId EntityId);
	
	FNetworkGUID GetNetGUIDFromUnrealObjectRef(const improbable::unreal::gdk::UnrealObjectRef& ObjectRef) const;
	improbable::unreal::gdk::UnrealObjectRef GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const;
	FNetworkGUID GetNetGUIDFromEntityId(worker::EntityId EntityId) const;

	void RegisterStaticObjects(const improbable::unreal::gdk::UnrealLevelData& LevelData);

	uint32 GetHashFromStaticClass(const UClass* StaticClass) const;
	UClass* GetStaticClassFromHash(uint32 Hash) const;

private:
	FNetworkGUID GetOrAssignNetGUID_SpatialGDK(const UObject* Object);
	void RegisterObjectRef(FNetworkGUID NetGUID, const improbable::unreal::gdk::UnrealObjectRef& ObjectRef);
	FNetworkGUID AssignStaticActorNetGUID(const UObject* Object, const FNetworkGUID& StaticNetGUID);

	void CreateStaticClassMapping();

	TMap<FNetworkGUID, FHashableUnrealObjectRef> NetGUIDToUnrealObjectRef;
	TMap<FHashableUnrealObjectRef, FNetworkGUID> UnrealObjectRefToNetGUID;
	TMap<uint32, UClass*> StaticClassHashMap;
};

