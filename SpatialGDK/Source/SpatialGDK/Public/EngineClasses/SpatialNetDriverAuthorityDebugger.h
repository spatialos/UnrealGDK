// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "SpatialConstants.h"

#include "SpatialNetDriverAuthorityDebugger.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialNetDriverAuthorityDebugger, Log, All);

class USpatialNetDriver;
class USpatialShadowActor;

/*
 * A helper object that checks invalid data modification on non-auth workers when using Spatial Networking
 */

UCLASS()
class SPATIALGDK_API USpatialNetDriverAuthorityDebugger : public UObject
{
	GENERATED_BODY()

public:
	USpatialNetDriverAuthorityDebugger() = default;

	void Init(USpatialNetDriver& InNetDriver);

	void AddSpatialShadowActor(const Worker_EntityId_Key EntityId);
	void RemoveSpatialShadowActor(const Worker_EntityId_Key EntityId);
	void UpdateSpatialShadowActor(const Worker_EntityId_Key EntityId);
	void CheckUnauthorisedDataChanges();

	static bool IsSuppressedActor(const AActor& Actor);
	static bool IsSuppressedProperty(const FProperty& Property, const AActor& Actor);

protected:
	UPROPERTY()
	USpatialNetDriver* NetDriver = nullptr;

	UPROPERTY()
	TMap<int64, USpatialShadowActor*> SpatialShadowActors;

	// Store a list of actors class names that currently violate the non-auth changes so that the user is not spammed.
	// TODO: link PR to investigate these cases
	const static TArray<FName> SuppressedActors;
	const static TArray<FName> SuppressedProperties;
	const static TArray<FName> SuppressedAutonomousProperties;
};
