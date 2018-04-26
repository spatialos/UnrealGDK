// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialOSBlueprintLibrary.h"
#include "SpatialGDKWorkerTypes.h"

USpatialOSBlueprintLibrary::USpatialOSBlueprintLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

bool USpatialOSBlueprintLibrary::EqualEqual_FEntityId(const FEntityId& A, const FEntityId& B)
{
  return A == B;
}

bool USpatialOSBlueprintLibrary::NotEqual_FEntityId(const FEntityId& A, const FEntityId& B)
{
  return A != B;
}

bool USpatialOSBlueprintLibrary::IsValid_FEntityId(const FEntityId& EntityId)
{
  return EntityId.ToSpatialEntityId() > 0;
}

FString USpatialOSBlueprintLibrary::EntityIdToString(const FEntityId& EntityId)
{
  return ToString(EntityId.ToSpatialEntityId());
}

FEntityIdQueryConstraint
USpatialOSBlueprintLibrary::MakeEntityIdQueryConstraintFromEntityId(const FEntityId& InEntityId)
{
  return FEntityIdQueryConstraint(InEntityId);
}

FEntityIdQueryConstraint
USpatialOSBlueprintLibrary::MakeEntityIdQueryConstraintFromInt(int InEntityId)
{
  return FEntityIdQueryConstraint(InEntityId);
}

FComponentIdQueryConstraint
USpatialOSBlueprintLibrary::MakeComponentIdQueryConstraintFromComponentId(
	const FComponentId& InComponentId)
{
  return FComponentIdQueryConstraint(InComponentId);
}

FComponentIdQueryConstraint
USpatialOSBlueprintLibrary::MakeComponentIdQueryConstraintFromInt(int InComponentId)
{
  return FComponentIdQueryConstraint(InComponentId);
}

FSphereQueryConstraint
USpatialOSBlueprintLibrary::MakeSphereQueryConstraint(const FVector& InPosition, float InRadius)
{
  return FSphereQueryConstraint(InPosition, InRadius);
}

UEntityQueryConstraint* USpatialOSBlueprintLibrary::SpatialOsEntityAndQuery(
	const TArray<FEntityIdQueryConstraint>& EntityIds,
	const TArray<FComponentIdQueryConstraint>& ComponentIds,
	const TArray<FSphereQueryConstraint>& Spheres)
{
  auto NewQuery = NewObject<UEntityQueryConstraint>();

  worker::query::AndConstraint WorkerConstraint;
  for (const FEntityIdQueryConstraint& constraint : EntityIds)
  {
	worker::query::EntityIdConstraint EntityIdConstraint;
	EntityIdConstraint.EntityId = constraint.Underlying.ToSpatialEntityId();
	WorkerConstraint.emplace_back(EntityIdConstraint);
  }

  for (const FComponentIdQueryConstraint& constraint : ComponentIds)
  {
	worker::query::ComponentConstraint ComponentConstraint;
	ComponentConstraint.ComponentId = constraint.Underlying.ToSpatialComponentId();
	WorkerConstraint.emplace_back(ComponentConstraint);
  }

  for (const FSphereQueryConstraint& constraint : Spheres)
  {
	worker::query::SphereConstraint SphereConstraint;
	SphereConstraint.Radius = static_cast<double>(constraint.Radius);
	SphereConstraint.X = static_cast<double>(constraint.Position.X);
	SphereConstraint.Y = static_cast<double>(constraint.Position.Y);
	SphereConstraint.Z = static_cast<double>(constraint.Position.Z);
	WorkerConstraint.emplace_back(SphereConstraint);
  }

  NewQuery->Underlying.emplace(WorkerConstraint);
  return NewQuery;
}

UEntityQueryConstraint* USpatialOSBlueprintLibrary::SpatialOsEntityOrQuery(
	const TArray<FEntityIdQueryConstraint>& EntityIds,
	const TArray<FComponentIdQueryConstraint>& ComponentIds,
	const TArray<FSphereQueryConstraint>& Spheres)
{
  auto NewQuery = NewObject<UEntityQueryConstraint>();

  worker::query::OrConstraint WorkerConstraint;
  for (const FEntityIdQueryConstraint& constraint : EntityIds)
  {
	worker::query::EntityIdConstraint EntityIdConstraint;
	EntityIdConstraint.EntityId = constraint.Underlying.ToSpatialEntityId();
	WorkerConstraint.emplace_back(EntityIdConstraint);
  }

  for (const FComponentIdQueryConstraint& constraint : ComponentIds)
  {
	worker::query::ComponentConstraint ComponentConstraint;
	ComponentConstraint.ComponentId = constraint.Underlying.ToSpatialComponentId();
	WorkerConstraint.emplace_back(ComponentConstraint);
  }

  for (const FSphereQueryConstraint& constraint : Spheres)
  {
	worker::query::SphereConstraint SphereConstraint;
	SphereConstraint.Radius = static_cast<double>(constraint.Radius);
	SphereConstraint.X = static_cast<double>(constraint.Position.X);
	SphereConstraint.Y = static_cast<double>(constraint.Position.Y);
	SphereConstraint.Z = static_cast<double>(constraint.Position.Z);
	WorkerConstraint.emplace_back(SphereConstraint);
  }

  NewQuery->Underlying.emplace(WorkerConstraint);
  return NewQuery;
}

bool USpatialOSBlueprintLibrary::IsSuccessful_Command(const FSpatialOSCommandResult& CommandResult)
{
  return CommandResult.StatusCode == ECommandResponseCode::Success;
}

ECommandResponseCode
USpatialOSBlueprintLibrary::GetCommandStatusCode(const FSpatialOSCommandResult& CommandResult)
{
  return CommandResult.StatusCode;
}

FString
USpatialOSBlueprintLibrary::GetCommandErrorMessage(const FSpatialOSCommandResult& CommandResult)
{
  return CommandResult.ErrorMessage;
}

bool USpatialOSBlueprintLibrary::EqualEqual_FRequestId(const FRequestId& A, const FRequestId& B)
{
  return A == B;
}

bool USpatialOSBlueprintLibrary::NotEqual_FRequestId(const FRequestId& A, const FRequestId& B)
{
  return A != B;
}

bool USpatialOSBlueprintLibrary::IsValid_FRequestId(const FRequestId& RequestId)
{
  return RequestId.IsValid();
}
