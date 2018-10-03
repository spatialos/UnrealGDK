// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

namespace improbable
{
struct Component
{
	virtual ~Component() {}

	bool bIsDynamic = false;
};

// TODO(nik): This should probable live somewhere else...
// Storage for arbitrary component data.
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
	explicit ComponentStorage(const typename T& data) : data{data} {}
	// TODO(nik): nowhere else is using 'std' - does this need to change?
	explicit ComponentStorage(typename T&& data) : data{std::move(data)} {}
	~ComponentStorage() override {}

	TUniquePtr<ComponentStorageBase> Copy() const override
	{
		return TUniquePtr<ComponentStorageBase>{new ComponentStorage{data}};
	}

	typename T& Get()
	{
		return data;
	}

private:
	typename T data;
};

}  // namespace improbable
