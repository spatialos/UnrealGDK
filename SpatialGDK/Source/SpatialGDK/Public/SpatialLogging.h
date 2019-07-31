// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Connection/SpatialWorkerConnection.h"
#include "Engine/World.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "GameFramework/Actor.h"
#include "SpatialConstants.h"

#define _SPATIALLOG_NETROLESTRING(Role) (Role == ROLE_SimulatedProxy ? TEXT("SimulatedProxy") : Role == ROLE_AutonomousProxy ? TEXT("AutonomousProxy") : Role == ROLE_Authority ? TEXT("Authority") : TEXT("None"))

/**
 * Standard UE_LOG that includes a prefix containing the worker ID, provided 'this->NetDriver' exists.
 */
#define UE_LOG_SPATIAL_IMPLICIT_NETDRIVER(CategoryName, Verbosity, Format, ...) \
{ \
	const class USpatialWorkerConnection* LocalConnection = [this] \
	{ \
		if (const class USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(this->NetDriver)) \
			return NetDriver->Connection; \
		return (USpatialWorkerConnection*)nullptr; \
	}(); \
	UE_LOG(CategoryName, Verbosity, TEXT("[%s] [%s] ") Format, \
		LocalConnection ? *LocalConnection->GetWorkerLabel() : TEXT("No Connection"), \
		*GetNameSafe(this), \
		##__VA_ARGS__); \
}

/**
 * Standard UE_LOG that includes a prefix containing the worker ID, authority, and entity information, provided 'this' is an AActor or derived type.
 */
#define UE_LOG_SPATIAL_THIS_ACTOR(CategoryName, Verbosity, Format, ...) UE_LOG_SPATIAL_ACTOR(this, CategoryName, Verbosity, Format, ##__VA_ARGS__) 

/**
 * Standard UE_LOG that includes a prefix containing the worker ID, authority, and entity information, provided 'Actor' is an AActor or derived type.
 */
#define UE_LOG_SPATIAL_ACTOR(Actor, CategoryName, Verbosity, Format, ...) \
{ \
	const AActor* SafeActor = (Actor); \
	const USpatialNetDriver* NetDriver = [SafeActor] \
	{ \
		if (SafeActor) \
			if (const UWorld* World = SafeActor->GetWorld()) \
				if (const USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(World->NetDriver)) \
					return NetDriver; \
		return (const USpatialNetDriver*)nullptr; \
	}(); \
	const int32 EntityId = NetDriver && NetDriver->PackageMap ? NetDriver->PackageMap->GetEntityIdFromObject(Actor) : SpatialConstants::INVALID_ENTITY_ID; \
	UE_LOG(CategoryName, Verbosity, TEXT("[%s] [L:%s R:%s] [%s ID:%d] ") Format, \
		NetDriver && NetDriver->Connection ? *NetDriver->Connection->GetWorkerLabel() : TEXT("No Connection"), \
		_SPATIALLOG_NETROLESTRING(SafeActor->GetLocalRole()), \
		_SPATIALLOG_NETROLESTRING(SafeActor->GetRemoteRole()), \
		*GetNameSafe(SafeActor), \
		EntityId, \
		##__VA_ARGS__); \
}
