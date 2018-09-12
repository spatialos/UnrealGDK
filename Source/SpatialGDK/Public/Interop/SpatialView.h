// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

#include "SpatialView.generated.h"

class USpatialNetDriver;
class USpatialReceiver;

UCLASS()
class SPATIALGDK_API USpatialView : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* NetDriver);
	void ProcessOps(Worker_OpList* OpList);

	Worker_Authority GetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	UnrealMetadata* GetUnrealMetadata(Worker_EntityId EntityId);

private:
	void OnAddComponent(const Worker_AddComponentOp& add_component);
	void OnRemoveEntity(const Worker_RemoveEntityOp& remove_entity);
	void OnAuthorityChange(const Worker_AuthorityChangeOp& Op);

	USpatialReceiver* Receiver;

	TMap<Worker_EntityId_Key, TMap<Worker_ComponentId, Worker_Authority>> EntityComponentAuthorityMap;
	TMap<Worker_EntityId_Key, TSharedPtr<UnrealMetadata>> EntityUnrealMetadataMap;
};
