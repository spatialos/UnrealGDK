// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EngineClasses/SpatialInterestConstraints.h"

#include "ActorInterestQueryComponent.generated.h"

namespace SpatialGDK
{
struct Query;
}
class USchemaDatabase;

/**
 * Creates a set of SpatialOS Queries for describing interest that this actor has in other entities.
 */
UCLASS(ClassGroup=(SpatialGDK), NotSpatialType, Meta=(BlueprintSpawnableComponent))
class SPATIALGDK_API UActorInterestQueryComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	UActorInterestQueryComponent();
	~UActorInterestQueryComponent() = default;

	void CreateQueries(const USchemaDatabase& SchemaDatabase, const SpatialGDK::QueryConstraint& AdditionalConstraints, TArray<SpatialGDK::Query>& OutQueries) const;

	/**
	 * Whether to use NetCullDistanceSquared to generate constraints relative to the Actor that this component is attached to.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	bool bUseNetCullDistanceForCheckoutRadius = true;

	/**
	 * The Queries associated with this component.
	 */
	UPROPERTY(BlueprintReadonly, EditDefaultsOnly)
	TArray<FQueryData> Queries;

};
