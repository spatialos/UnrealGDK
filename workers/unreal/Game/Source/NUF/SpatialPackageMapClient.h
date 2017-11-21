// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/PackageMapClient.h"
#include "SpatialPackageMapClient.generated.h"

/**
 * 
 */
UCLASS()
class NUF_API USpatialPackageMapClient : public UPackageMapClient
{
	GENERATED_BODY()		
public:
	void ResolveStaticObjectGUID(FNetworkGUID& GUID, FString& Path);
};


class NUF_API FSpatialNetGUIDCache : public FNetGUIDCache
{
public:
	FSpatialNetGUIDCache(class USpatialNetDriver* InDriver);

	// requires this to be made virtual in UPackageMapClient.h
	FNetworkGUID AssignNewNetGUID_Server(const UObject* Object) override;

	void RegisterPreallocatedNetGUID(const FNetworkGUID& NetGUID, const UObject* Object, const FString& Path);

	TMap<FNetworkGUID, int64> NetGuidToEntityIdMap;
};
