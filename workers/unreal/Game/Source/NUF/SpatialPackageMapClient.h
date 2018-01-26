// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/PackageMapClient.h"
#include "EntityId.h"
#include <unreal/core_types.h>

#include "SpatialPackageMapClient.generated.h"

class USpatialActorChannel;

//todo-giray: super hacky to inject GetTypeHash() into UnrealObjectRef. Will find a better way.
class FUnrealObjectRefWrapper
{
public:
	improbable::unreal::UnrealObjectRef ObjectRef;
	bool operator == (const FUnrealObjectRefWrapper& Rhs) const
	{
		return ObjectRef == Rhs.ObjectRef;
	}
	friend uint32 GetTypeHash(const FUnrealObjectRefWrapper& ObjectRefWrapper)
	{
		//todo-giray do a proper hash.
		return (ObjectRefWrapper.ObjectRef.entity() << 8) + ObjectRefWrapper.ObjectRef.offset();
	}
};

struct FPendingObjRefData
{
	FPendingObjRefData() = delete;
	FPendingObjRefData(USpatialActorChannel* InChannel)
		: Channel(InChannel)
	{}

	USpatialActorChannel* Channel;
	TArray<uint16> Handles;
};

/**
 * 
 */
UCLASS()
class NUF_API USpatialPackageMapClient : public UPackageMapClient
{
	GENERATED_BODY()		
public:
	void ResolveStaticObjectGUID(FNetworkGUID& NetGUID, FString& Path);
	FNetworkGUID ResolveEntityActor(AActor* Actor, FEntityId EntityId);
	virtual bool SerializeNewActor(FArchive& Ar, class UActorChannel *Channel, class AActor*& Actor) override;

	improbable::unreal::UnrealObjectRef GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const;
	FNetworkGUID GetNetGUIDFromUnrealObjectRef(const improbable::unreal::UnrealObjectRef& ObjectRef) const;
	FNetworkGUID GetNetGUIDFromEntityId(const worker::EntityId& EntityId) const;
	void AddPendingObjRef(UObject* Object, USpatialActorChannel* DependentChannel, uint16 Handle);
};

class NUF_API FSpatialNetGUIDCache : public FNetGUIDCache
{
public:
	FSpatialNetGUIDCache(class USpatialNetDriver* InDriver);
		
	FNetworkGUID AssignNewEntityActorNetGUID(AActor* Actor);
	
	FNetworkGUID GetNetGUIDFromUnrealObjectRef(const improbable::unreal::UnrealObjectRef& ObjectRef) const;
	improbable::unreal::UnrealObjectRef FSpatialNetGUIDCache::GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const;
	FNetworkGUID GetNetGUIDFromEntityId(const worker::EntityId& EntityId) const;

	void AddPendingObjRef(UObject* Object, USpatialActorChannel* DependentChannel, uint16 Handle);	
private:
	FNetworkGUID AssignNewNetGUID(const UObject* Object);
	void ResolvePendingObjRefs(const UObject* Object);

	TMap<FNetworkGUID, FUnrealObjectRefWrapper> NetGUIDToUnrealObjectRef;
	TMap<FUnrealObjectRefWrapper, FNetworkGUID> UnrealObjectRefToNetGUID;

	TMap<UObject*, TArray<USpatialActorChannel*>> ChannelsAwaitingObjRefResolve;
	TMap<USpatialActorChannel*, TArray<uint16>> PendingObjRefHandles;
};

