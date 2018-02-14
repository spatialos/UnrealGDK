// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/PackageMapClient.h"
#include "EntityId.h"
#include "SpatialUnrealObjectRef.h"
#include "SpatialInterop.h"

#include <improbable/unreal/level_data.h>

#include "SpatialPackageMapClient.generated.h"

class USpatialActorChannel;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOSPackageMap, Log, All);

/**
 * 
 */
UCLASS()
class NUF_API USpatialPackageMapClient : public UPackageMapClient
{
	GENERATED_BODY()		
public:
	//void ResolveStaticObjectGUID(FNetworkGUID& NetGUID, FString& Path);
	FNetworkGUID ResolveEntityActor(AActor* Actor, FEntityId EntityId);
	virtual bool SerializeNewActor(FArchive& Ar, class UActorChannel *Channel, class AActor*& Actor) override;

	improbable::unreal::UnrealObjectRef GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const;
	FNetworkGUID GetNetGUIDFromUnrealObjectRef(const improbable::unreal::UnrealObjectRef& ObjectRef) const;
	FNetworkGUID GetNetGUIDFromEntityId(const worker::EntityId& EntityId) const;

	void RegisterStaticObjects(const improbable::unreal::UnrealLevelData& LevelData);
};

class NUF_API FSpatialNetGUIDCache : public FNetGUIDCache
{
public:
	FSpatialNetGUIDCache(class USpatialNetDriver* InDriver);
		
	FNetworkGUID AssignNewEntityActorNetGUID(AActor* Actor);
	
	FNetworkGUID GetNetGUIDFromUnrealObjectRef(const improbable::unreal::UnrealObjectRef& ObjectRef) const;
	improbable::unreal::UnrealObjectRef GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const;
	FNetworkGUID GetNetGUIDFromEntityId(const worker::EntityId& EntityId) const;

	void RegisterStaticObjects(const improbable::unreal::UnrealLevelData& LevelData);

private:
	//FNetworkGUID AssignNewNetGUID(const UObject* Object);
	FNetworkGUID GetOrAssignNetGUID_NUF(const UObject* Object);
	void RegisterObjectRef(FNetworkGUID NetGUID, const improbable::unreal::UnrealObjectRef& ObjectRef);
	FNetworkGUID AssignStaticActorNetGUID(const UObject* Object, const FNetworkGUID& StaticNetGUID);

	TMap<FNetworkGUID, FHashableUnrealObjectRef> NetGUIDToUnrealObjectRef;
	TMap<FHashableUnrealObjectRef, FNetworkGUID> UnrealObjectRefToNetGUID;
};

