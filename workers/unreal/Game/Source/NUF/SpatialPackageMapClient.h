// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/PackageMapClient.h"
#include "EntityId.h"
#include <unreal/core_types.h>

#include "SpatialPackageMapClient.generated.h"

/**
 * 
 */
UCLASS()
class NUF_API USpatialPackageMapClient : public UPackageMapClient
{
	GENERATED_BODY()		
public:
	void ResolveStaticObjectGUID(FNetworkGUID& NetGUID, FString& Path);
	void ResolveEntityActor(AActor* Actor, FEntityId EntityId);
	virtual bool SerializeNewActor(FArchive& Ar, class UActorChannel *Channel, class AActor*& Actor) override;
};

class NUF_API FSpatialNetGUIDCache : public FNetGUIDCache
{
public:
	FSpatialNetGUIDCache(class USpatialNetDriver* InDriver);

	//NUF-sourcechange Make virtual in PackageMapClient.h
	FNetworkGUID AssignNewNetGUID_Server(const UObject* Object) override;

	FNetworkGUID AssignNewEntityActorNetGUID(AActor* Actor);
	
	FNetworkGUID GetNetGUIDFromUnrealObjectRef(const improbable::unreal::UnrealObjectRef& ObjectRef);
	improbable::unreal::UnrealObjectRef FSpatialNetGUIDCache::GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID);
	FNetworkGUID GetNetGUIDFromEntityId(const worker::EntityId& EntityId);
private:
	FNetworkGUID AssignNewNetGUID(const UObject* Object);

	TMap<FNetworkGUID, improbable::unreal::UnrealObjectRef> NetGUIDToUnrealObjectRef;
	TMap<improbable::unreal::UnrealObjectRef, FNetworkGUID> UnrealObjectRefToNetGUID;
};

uint32 GetTypeHash(const improbable::unreal::UnrealObjectRef& ObjectRef)
{
	//todo-giray do a proper hash.
	return (ObjectRef.entity() << 8) + ObjectRef.offset();
}