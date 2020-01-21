// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
 
#pragma once
 
#include "Core.h"
 
#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

class USpatialActorChannel;
class USpatialNetDriver;
class USpatialPackageMap;
class USpatialClassInfoManager;
class USpatialPackageMapClient;
 
namespace SpatialGDK
{
class InterestFactory;
class SpatialRPCService;	

struct RPCsOnEntityCreation;
using FRPCsOnEntityCreationMap = TMap<TWeakObjectPtr<const UObject>, RPCsOnEntityCreation>;
 
class SPATIALGDK_API EntityFactory
{
public:
	EntityFactory(USpatialNetDriver* InNetDriver, USpatialPackageMapClient* InPackageMap, USpatialClassInfoManager* InClassInfoManager, SpatialRPCService* InRPCService, const InterestFactory* InInterestFactory);
 
	TArray<Worker_ComponentData> CreateEntityComponents(USpatialActorChannel* Channel, FRPCsOnEntityCreationMap& OutgoingOnCreateEntityRPCs);
 
private:
	USpatialNetDriver* NetDriver;
	USpatialPackageMapClient* PackageMap;
	USpatialClassInfoManager* ClassInfoManager;
	SpatialRPCService* RPCService;
	const InterestFactory* SpatialInterestFactory;
};
}
