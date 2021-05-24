// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
#include "Utils/RepDataUtils.h"

DECLARE_LOG_CATEGORY_EXTERN(LogClientNetLoadActorHelper, Log, All);

class USpatialNetDriver;

namespace SpatialGDK
{

class FClientNetLoadActorHelper
{
public:
	FClientNetLoadActorHelper(USpatialNetDriver& InNetDriver);
	void EntityRemoved(const Worker_EntityId EntityId, const AActor* Actor);
	UObject* GetReusableDynamicSubObject(const FUnrealObjectRef ObjectRef);
	void RemoveRuntimeRemovedComponents(const Worker_EntityId& EntityId,
														const TArray<ComponentData>& NewComponents);
private:
	USpatialNetDriver* NetDriver;

	// Stores subobjects from client net load actors that have gone out of the client's interest
	TMap<Worker_EntityId_Key, TMap<FUnrealObjectRef, FNetworkGUID>> EntityRemovedDynamicSubObjects;

	// BNetLoadOnClient component edge case handling
	FNetworkGUID* GetSavedDynamicSubObjectNetGUID(const FUnrealObjectRef& ObjectRef);
	void SaveDynamicSubObjectRef(const FUnrealObjectRef& ObjectRef, const FNetworkGUID& NetGUID);
	void ClearDynamicSubObjectRefs(const Worker_EntityId& InEntityId);
};

} // namespace SpatialGDK
