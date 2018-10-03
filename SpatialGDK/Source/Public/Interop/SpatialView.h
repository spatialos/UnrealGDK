// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Schema/Component.h"
#include "Schema/StandardLibrary.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

#include "SpatialView.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialView, Log, All);

class USpatialNetDriver;
class USpatialReceiver;
class USpatialSender;

UCLASS()
class SPATIALGDK_API USpatialView : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* NetDriver);
	void ProcessOps(Worker_OpList* OpList);

	Worker_Authority GetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	improbable::UnrealMetadata* GetUnrealMetadata(Worker_EntityId EntityId);
	improbable::EntityAcl* GetEntityACL(Worker_EntityId EntityId);
	template <typename T> typename  T* GetComponentData(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId);

private:
	// TODO(nik): Helper method to get list of components via entity id?
	void OnAddComponent(const Worker_AddComponentOp& Op);
	void OnRemoveEntity(const Worker_RemoveEntityOp& Op);
	void OnComponentUpdate(const Worker_ComponentUpdateOp& Op);
	void OnAuthorityChange(const Worker_AuthorityChangeOp& Op);

	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	USpatialReceiver* Receiver;

	UPROPERTY()
	USpatialSender* Sender;

	// TODO(nik): Merge this map with the component map
	TMap<Worker_EntityId_Key, TMap<Worker_ComponentId, Worker_Authority>> EntityComponentAuthorityMap;

	// TODO(nik): Should this be a shared pointer or a unique pointer?
	TMap<Worker_EntityId_Key, TMap<Worker_ComponentId, TSharedPtr<improbable::ComponentStorageBase>>> EntityComponentMap;
};
