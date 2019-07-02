// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialInterestConstraints.generated.h"

namespace SpatialGDK
{
struct QueryConstraint;
}
class USchemaDatabase;

UCLASS(Abstract, BlueprintInternalUseOnly)
class SPATIALGDK_API UAbstractQueryConstraint : public UObject
{
	GENERATED_BODY()
public:
	UAbstractQueryConstraint() = default;
	virtual ~UAbstractQueryConstraint() = default;

	virtual void CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const PURE_VIRTUAL(UAbstractQueryConstraint::CreateConstraint, );
};

/**
 * Creates a constraint that is satisfied if any of its inner constraints are satisfied.
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SPATIALGDK_API UOrConstraint final : public UAbstractQueryConstraint
{
	GENERATED_BODY()
public:
	UOrConstraint() = default;
	~UOrConstraint() = default;

	virtual void CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const override;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Instanced)
	TArray<UAbstractQueryConstraint *> Constraints;
};

/**
 * Creates a constraint that is satisfied if all of its inner constraints are satisfied.
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SPATIALGDK_API UAndConstraint final : public UAbstractQueryConstraint
{
	GENERATED_BODY()
public:
	UAndConstraint() = default;
	~UAndConstraint() = default;

	virtual void CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const override;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Instanced)
	TArray<UAbstractQueryConstraint *> Constraints;
};

/**
 * Creates a constraint that includes all entities within a sphere centered on the specified point.
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SPATIALGDK_API USphereConstraint final : public UAbstractQueryConstraint
{
	GENERATED_BODY()
public:
	USphereConstraint() = default;
	~USphereConstraint() = default;

	virtual void CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const override;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FVector Center = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Meta = (ClampMin = 0.0))
	float Radius = 0.0f;
};

/**
 * Creates a constraint that includes all entities within a cylinder centered on the specified point.
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SPATIALGDK_API UCylinderConstraint final : public UAbstractQueryConstraint
{
	GENERATED_BODY()
public:
	UCylinderConstraint() = default;
	~UCylinderConstraint() = default;

	virtual void CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const override;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FVector Center = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Meta = (ClampMin = 0.0))
	float Radius = 0.0f;
};

/**
 * Creates a constraint that includes all entities within a bounding box centered on the specified point.
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SPATIALGDK_API UBoxConstraint final : public UAbstractQueryConstraint
{
	GENERATED_BODY()
public:
	UBoxConstraint() = default;
	~UBoxConstraint() = default;

	virtual void CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const override;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FVector Center = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Meta = (ClampMin = 0.0))
	FVector EdgeLengths = FVector::ZeroVector;
};

/**
 * Creates a constraint that includes all entities within a sphere centered on the actor.
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SPATIALGDK_API URelativeSphereConstraint final : public UAbstractQueryConstraint
{
	GENERATED_BODY()
public:
	URelativeSphereConstraint() = default;
	~URelativeSphereConstraint() = default;

	virtual void CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const override;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Meta = (ClampMin = 0.0))
	float Radius = 0.0f;
};

/**
 * Creates a constraint that includes all entities within a cylinder centered on the actor.
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SPATIALGDK_API URelativeCylinderConstraint final : public UAbstractQueryConstraint
{
	GENERATED_BODY()
public:
	URelativeCylinderConstraint() = default;
	~URelativeCylinderConstraint() = default;

	virtual void CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const override;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Meta = (ClampMin = 0.0))
	float Radius = 0.0f;
};

/**
 * Creates a constraint that includes all entities within a bounding box centered on the actor.
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SPATIALGDK_API URelativeBoxConstraint final : public UAbstractQueryConstraint
{
	GENERATED_BODY()
public:
	URelativeBoxConstraint() = default;
	~URelativeBoxConstraint() = default;

	virtual void CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const override;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Meta = (ClampMin = 0.0))
	FVector EdgeLengths = FVector::ZeroVector;
};

/**
 * Creates a constraint that includes an actor type (including subtypes) and a cylindrical range around the actor with the interest.
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SPATIALGDK_API UCheckoutRadiusConstraint final : public UAbstractQueryConstraint
{
	GENERATED_BODY()
public:
	UCheckoutRadiusConstraint() = default;
	~UCheckoutRadiusConstraint() = default;

	virtual void CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const override;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Meta = (ClampMin = 0.0))
	float Radius = 0.0f;
};

/**
 * Creates a constraint that includes all actors of a type (optionally including subtypes).
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SPATIALGDK_API UActorClassConstraint final : public UAbstractQueryConstraint
{
	GENERATED_BODY()
public:
	UActorClassConstraint() = default;
	~UActorClassConstraint() = default;

	virtual void CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const override;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	bool bIncludeDerivedClasses = true;
};

/**
 * Creates a constraint that includes all components of a type (optionally including subtypes).
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SPATIALGDK_API UComponentClassConstraint final : public UAbstractQueryConstraint
{
	GENERATED_BODY()
public:
	UComponentClassConstraint() = default;
	~UComponentClassConstraint() = default;

	virtual void CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const override;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TSubclassOf<UActorComponent> ComponentClass;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	bool bIncludeDerivedClasses = true;
};






