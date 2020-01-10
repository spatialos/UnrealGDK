// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "Interop/AbstractStaticComponentView.h"
#include "WorkerSDK/improbable/c_worker.h"

#include "Containers/Map.h"
#include "Templates/UniquePtr.h"

#include "SpatialStaticComponentViewMock.generated.h"

UCLASS()
class USpatialStaticComponentViewMock : public UObject, public AbstractStaticComponentView
{
	GENERATED_BODY()
public:

	void Init(TMap<Worker_EntityId_Key, TMap<Worker_ComponentId, Worker_Authority>> InEntityComponentAuthorityMap,
		TMap<Worker_EntityId_Key, TMap<Worker_ComponentId, TUniquePtr<SpatialGDK::ComponentStorageBase>>> InEntityComponentMap);

	virtual bool HasAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const override;
	virtual Worker_Authority GetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const override;
	virtual SpatialGDK::ComponentStorageBase* GetComponentData(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const;

private:
	TMap<Worker_EntityId_Key, TMap<Worker_ComponentId, Worker_Authority>> EntityComponentAuthorityMap;
	TMap<Worker_EntityId_Key, TMap<Worker_ComponentId, TUniquePtr<SpatialGDK::ComponentStorageBase>>> EntityComponentMap;
}; 
