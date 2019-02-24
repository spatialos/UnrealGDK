// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Schema/Component.h"
#include "Schema/StandardLibrary.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialStaticComponentView.generated.h"

UCLASS()
class SPATIALGDK_API USpatialStaticComponentView : public UObject
{
	GENERATED_BODY()

public:
	Worker_Authority GetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	bool HasAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId);

	template <typename T>
	T* GetComponentData(Worker_EntityId EntityId)
	{
		if (TMap<Worker_ComponentId, TUniquePtr<improbable::ComponentStorageBase>>* ComponentStorageMap = EntityComponentMap.Find(EntityId))
		{
			if (TUniquePtr<improbable::ComponentStorageBase>* Component = ComponentStorageMap->Find(T::ComponentId))
			{
				return &(static_cast<improbable::ComponentStorage<T>*>(Component->Get())->Get());
			}
		}

		return nullptr;
	}

	void OnAddComponent(const Worker_AddComponentOp& Op);
	void OnRemoveEntity(Worker_EntityId EntityId);
	void OnComponentUpdate(const Worker_ComponentUpdateOp& Op);
	void OnAuthorityChange(const Worker_AuthorityChangeOp& Op);

private:
	TMap<Worker_EntityId_Key, TMap<Worker_ComponentId, Worker_Authority>> EntityComponentAuthorityMap;
	TMap<Worker_EntityId_Key, TMap<Worker_ComponentId, TUniquePtr<improbable::ComponentStorageBase>>> EntityComponentMap;
};
