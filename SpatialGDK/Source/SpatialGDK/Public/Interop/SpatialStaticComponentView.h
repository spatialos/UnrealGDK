// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "Schema/StandardLibrary.h"
#include "SpatialConstants.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "Containers/Map.h"
#include "Templates/UniquePtr.h"
#include "UObject/Object.h"

#include "SpatialStaticComponentView.generated.h"

UCLASS()
class SPATIALGDK_API USpatialStaticComponentView : public UObject
{
	GENERATED_BODY()

public:
	bool HasAuthority(FEntityId EntityId, Worker_ComponentId ComponentId) const;

	template <typename T>
	T* GetComponentData(FEntityId EntityId) const
	{
		if (const auto* ComponentStorageMap = EntityComponentMap.Find(EntityId))
		{
			if (const TUniquePtr<SpatialGDK::Component>* Component = ComponentStorageMap->Find(T::ComponentId))
			{
				return static_cast<T*>(Component->Get());
			}
		}

		return nullptr;
	}

	bool HasComponent(FEntityId EntityId, Worker_ComponentId ComponentId) const;

	void OnAddComponent(const Worker_AddComponentOp& Op);
	void OnRemoveComponent(const Worker_RemoveComponentOp& Op);
	void OnRemoveEntity(FEntityId EntityId);
	void OnComponentUpdate(const Worker_ComponentUpdateOp& Op);
	void OnAuthorityChange(const Worker_AuthorityChangeOp& Op);

	void GetEntityIds(TArray<FEntityId>& OutEntityIds) const { EntityComponentMap.GetKeys(OutEntityIds); }

private:
	Worker_Authority GetAuthority(FEntityId EntityId, Worker_ComponentId ComponentId) const;

	TMap<FEntityId, TMap<Worker_ComponentId, Worker_Authority>> EntityComponentAuthorityMap;
	TMap<FEntityId, TMap<Worker_ComponentId, TUniquePtr<SpatialGDK::Component>>> EntityComponentMap;
};
