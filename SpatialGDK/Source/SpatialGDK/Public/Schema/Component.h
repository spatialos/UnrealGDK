// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <WorkerSDK/improbable/c_worker.h>
#include "CoreMinimal.h"

namespace SpatialGDK
{

struct Component
{
	virtual ~Component() {}
	virtual void ApplyComponentUpdate(const Worker_ComponentUpdate& Update) {}
};

class ComponentStorageBase
{
public:
	virtual ~ComponentStorageBase(){};
	virtual TUniquePtr<ComponentStorageBase> Copy() const = 0;
};

template <typename T>
static T* GetComponentStorageData(SpatialGDK::ComponentStorageBase* StorageBase)
{
	return &(static_cast<SpatialGDK::ComponentStorage<T>*>(StorageBase)->Get());
}

template <typename T>
class ComponentStorage : public ComponentStorageBase
{
public:
	explicit ComponentStorage(const T& data) : data{data} {}
	explicit ComponentStorage(T&& data) : data{MoveTemp(data)} {}
	~ComponentStorage() override {}

	TUniquePtr<ComponentStorageBase> Copy() const override
	{
		return TUniquePtr<ComponentStorageBase>{new ComponentStorage{data}};
	}

	T& Get()
	{
		return data;
	}

private:
	T data;
};

} // namespace SpatialGDK
