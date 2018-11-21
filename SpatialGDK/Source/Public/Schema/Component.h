// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <utility>
#include <WorkerSDK/improbable/c_worker.h>

namespace improbable
{

struct Component
{
	virtual ~Component() {}
	virtual void ApplyComponentUpdate(const Worker_ComponentUpdate& Update) {}

	bool bIsDynamic = false;
};

class ComponentStorageBase
{
public:
	virtual ~ComponentStorageBase(){};
	virtual TUniquePtr<ComponentStorageBase> Copy() const = 0;
};

template <typename T>
class ComponentStorage : public ComponentStorageBase
{
public:
	explicit ComponentStorage(const T& InData) : Data{ InData } {}
	explicit ComponentStorage(T&& InData) : Data{std::move(InData)} {}
	~ComponentStorage() override {}

	TUniquePtr<ComponentStorageBase> Copy() const override
	{
		return TUniquePtr<ComponentStorageBase>{new ComponentStorage{ Data }};
	}

	T& Get()
	{
		return Data;
	}

private:
	T Data;
};

}
