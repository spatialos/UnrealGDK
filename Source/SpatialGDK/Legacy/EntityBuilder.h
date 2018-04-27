#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKWorkerTypes.h"
#include "improbable/standard_library.h"

namespace improbable
{
namespace unreal
{
class FEntityComponentBuilder
{
  private:
	friend class FGenericComponentBuilder;
	friend class FReadAclComponentBuilder;
	friend class FPersistenceComponentBuilder;
	friend class FMetadataComponentBuilder;
	friend class FPositionComponentBuilder;

	FEntityComponentBuilder(
		worker::Entity Entity,
		worker::Map<std::uint32_t, improbable::WorkerRequirementSet>
			WriteRequirement,
		improbable::WorkerRequirementSet ReadRequirement)
	: InternalEntity(std::move(Entity)), ComponentAuthority(std::move(WriteRequirement)), ReadRequirementSet(std::move(ReadRequirement))
	{
	}

	FEntityComponentBuilder()
	: ReadRequirementSet{{}}
	{
	}

  public:
	FEntityComponentBuilder(FEntityComponentBuilder&& rhs) = default;

  protected:
	worker::Entity InternalEntity;

	worker::Map<std::uint32_t, improbable::WorkerRequirementSet>
		ComponentAuthority;
	improbable::WorkerRequirementSet ReadRequirementSet;
};

class FGenericComponentBuilder : public FEntityComponentBuilder
{
  private:
	friend class FReadAclComponentBuilder;
	FGenericComponentBuilder(
		worker::Entity Entity,
		worker::Map<std::uint32_t, improbable::WorkerRequirementSet>
			WriteRequirement,
		improbable::WorkerRequirementSet ReadRequirement)
	: FEntityComponentBuilder(std::move(Entity), std::move(WriteRequirement), std::move(ReadRequirement))
	{
	}

	FGenericComponentBuilder(FGenericComponentBuilder&& rhs) = default;

  public:
	FGenericComponentBuilder() = delete;
	FGenericComponentBuilder(const FGenericComponentBuilder& rhs) = delete;

	template <class T>
	FGenericComponentBuilder
	AddComponent(const typename T::Data& data,
		const improbable::WorkerRequirementSet& WriteRequirement)
	{
		InternalEntity.Add<T>(data);
		ComponentAuthority.emplace(T::ComponentId, WriteRequirement);
		return FGenericComponentBuilder(InternalEntity, ComponentAuthority, ReadRequirementSet);
	}

	FGenericComponentBuilder SetEntityAclComponentWriteAccess(
		const improbable::WorkerRequirementSet& WriteRequirement)
	{
		ComponentAuthority.emplace(improbable::EntityAcl::ComponentId,
			WriteRequirement);
		return FGenericComponentBuilder(InternalEntity, ComponentAuthority, ReadRequirementSet);
	}

	worker::Entity Build()
	{
		InternalEntity.Add<improbable::EntityAcl>(
			improbable::EntityAcl::Data(ReadRequirementSet, ComponentAuthority));
		return InternalEntity;
	}
};

class FReadAclComponentBuilder : public FEntityComponentBuilder
{
  private:
	friend class FPersistenceComponentBuilder;
	FReadAclComponentBuilder(
		worker::Entity Entity,
		worker::Map<std::uint32_t, improbable::WorkerRequirementSet>
			WriteRequirement,
		improbable::WorkerRequirementSet ReadRequirement)
	: FEntityComponentBuilder(std::move(Entity), std::move(WriteRequirement), std::move(ReadRequirement))
	{
	}

	FReadAclComponentBuilder(FReadAclComponentBuilder&& rhs)
	: FEntityComponentBuilder(std::move(rhs.InternalEntity),
		  std::move(rhs.ComponentAuthority),
		  std::move(rhs.ReadRequirementSet))
	{
	}

  public:
	FReadAclComponentBuilder() = delete;
	FReadAclComponentBuilder(const FReadAclComponentBuilder& rhs) = delete;

	FGenericComponentBuilder
	SetReadAcl(const improbable::WorkerRequirementSet& ReadRequirement)
	{
		ReadRequirementSet = ReadRequirement;
		return FGenericComponentBuilder(InternalEntity, ComponentAuthority, ReadRequirementSet);
	}
};

class FPersistenceComponentBuilder : public FEntityComponentBuilder
{
  private:
	friend class FMetadataComponentBuilder;
	FPersistenceComponentBuilder(
		worker::Entity Entity,
		worker::Map<std::uint32_t, improbable::WorkerRequirementSet>
			WriteRequirement,
		improbable::WorkerRequirementSet ReadRequirement)
	: FEntityComponentBuilder(std::move(Entity), std::move(WriteRequirement), std::move(ReadRequirement))
	{
	}

	FPersistenceComponentBuilder(FPersistenceComponentBuilder&& rhs) = default;

  public:
	FPersistenceComponentBuilder() = delete;
	FPersistenceComponentBuilder(const FPersistenceComponentBuilder& rhs) =
		delete;

	FReadAclComponentBuilder SetPersistence(bool IsPersistent)
	{
		if (IsPersistent)
		{
			InternalEntity.Add<improbable::Persistence>(
				improbable::Persistence::Data{});
		}
		return FReadAclComponentBuilder(InternalEntity, ComponentAuthority, ReadRequirementSet);
	}
};

class FMetadataComponentBuilder : public FEntityComponentBuilder
{
  private:
	friend class FPositionComponentBuilder;
	FMetadataComponentBuilder(
		worker::Entity Entity,
		worker::Map<std::uint32_t, improbable::WorkerRequirementSet>
			WriteRequirement,
		improbable::WorkerRequirementSet ReadRequirement)
	: FEntityComponentBuilder(std::move(Entity), std::move(WriteRequirement), std::move(ReadRequirement))
	{
	}

	FMetadataComponentBuilder(FMetadataComponentBuilder&& rhs) = default;

  public:
	FMetadataComponentBuilder() = delete;
	FMetadataComponentBuilder(const FMetadataComponentBuilder& rhs) = delete;

	FPersistenceComponentBuilder
	AddMetadataComponent(const improbable::Metadata::Data& data)
	{
		InternalEntity.Add<improbable::Metadata>(data);
		return FPersistenceComponentBuilder(InternalEntity, ComponentAuthority, ReadRequirementSet);
	}
};

class FPositionComponentBuilder : public FEntityComponentBuilder
{
  private:
	friend class FEntityBuilder;
	FPositionComponentBuilder()
	{
	}
	FPositionComponentBuilder(FPositionComponentBuilder&& rhs) = default;

  public:
	FPositionComponentBuilder(const FPositionComponentBuilder& rhs) = delete;

	FMetadataComponentBuilder AddPositionComponent(
		const improbable::Position::Data& data,
		const improbable::WorkerRequirementSet& WriteRequirement)
	{
		InternalEntity.Add<improbable::Position>(data);
		ComponentAuthority.emplace(improbable::Position::ComponentId,
			WriteRequirement);
		return FMetadataComponentBuilder(InternalEntity, ComponentAuthority, ReadRequirementSet);
	}
};

/**
*  Helper class for building a SpatialOS worker::Entity. Call Begin() to use.
*
*  Remarks:
*  FEntityBuilder provides a builder for creating worker::Entity objects easily.
* Its helper
*  methods are structured such that all required components must be added before
* an Entity can be
* constructed.
*
*  Helpers must be called in the following order:
*  Begin,
*  AddPositionComponent,
*  AddMetadataComponent,
*  SetPersistence,
*  SetReadAcl.
*  After this point, all required components will have been set. More components
* can be added with
*  AddComponent<>, and the worker::Entity can be built with Build.
*
*  Example:
*  auto snapshotEntity = FEntityBuilder::Begin()
*  .AddPositionComponent(Position::Data{ initialPosition }, writeACL)
*  .AddMetadataComponent(Metadata::Data{ "Npc" })
*  .SetPersistence(true)
*  .SetReadAcl(readACL)
*  .SetAclWriteAuthority(writeACL)
*  .AddComponent<T>(T::Data{}, writeACL)
*  .Build();
*/

class FEntityBuilder
{
  public:
	static FPositionComponentBuilder Begin()
	{
		return FPositionComponentBuilder();
	}
};
}  // ::unreal
}  // ::improbable
