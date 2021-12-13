// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/SpawnData.h"
#include "Schema/UnrealMetadata.h"
#include "Utils/RepDataUtils.h"

#include "Interop/ClientNetLoadActorHelper.h"
#include "Interop/SpatialCommandsHandler.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLightweightActorSystem, Log, All);

class USpatialNetDriver;

namespace SpatialGDK
{
class FSubView;

struct ActorData
{
	SpawnData Spawn;
	UnrealMetadata Metadata;
};

class LightweightActorSystem
{
public:
	LightweightActorSystem(const FSubView& InLightweightActorSubView, USpatialNetDriver* InNetDriver);

	void Advance();

	void EntityAdded(const Worker_EntityId EntityId);
	void EntityRemoved(const Worker_EntityId EntityId);
	void RefreshEntity(const Worker_EntityId EntityId);


private:
	void PopulateDataStore(const Worker_EntityId EntityId);

	// Deserialized state store for Actor relevant components.
	TMap<Worker_EntityId_Key, ActorData> LightweightActorDataStore;

	const FSubView* LightweightActorSubView;
	USpatialNetDriver* NetDriver;
};
} // namespace SpatialGDK
