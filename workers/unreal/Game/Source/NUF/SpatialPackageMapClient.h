// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/PackageMapClient.h"
#include "EntityId.h"
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
};


class NUF_API FSpatialNetGUIDCache : public FNetGUIDCache
{
public:
	FSpatialNetGUIDCache(class USpatialNetDriver* InDriver);

	//NUF-sourcechange Make virtual in PackageMapClient.h
	FNetworkGUID AssignNewNetGUID_Server(const UObject* Object) override;

	FNetworkGUID AssignNewEntityActorNetGUID(AActor* Actor);

	FEntityId GetEntityIdFromNetGUID(const FNetworkGUID NetGUID);
	FNetworkGUID GetNetGUIDFromEntityId(const FEntityId EntityId);

private:

	FNetworkGUID AssignNewNetGUID(const UObject* Object);

	UPROPERTY()
	TMap<FNetworkGUID, FEntityId> NetGUIDToEntityIdMap;
	UPROPERTY()
	TMap<FEntityId, FNetworkGUID> EntityIdToNetGUIDMap;
};
