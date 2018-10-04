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
	template <typename T> typename  T* GetComponentData(Worker_EntityId EntityId);

private:
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

	TMap<Worker_EntityId, TMap<Worker_ComponentId, Worker_Authority>> EntityComponentAuthorityMap;
	TMap<Worker_EntityId, TMap<Worker_ComponentId, TSharedPtr<improbable::ComponentStorageBase>>> EntityComponentMap;
};
