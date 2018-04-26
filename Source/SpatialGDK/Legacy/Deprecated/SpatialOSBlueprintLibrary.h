// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "ComponentId.h"
#include "CoreMinimal.h"
#include "EntityId.h"
#include "EntityQueryConstraints.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SpatialOSCommandResult.h"
#include "SpatialOSBlueprintLibrary.generated.h"

UCLASS()
class SPATIALGDK_API USpatialOSBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal (EntityId)", CompactNodeTitle = "=="), Category = "SpatialOS EntityId")
	static bool EqualEqual_FEntityId(const FEntityId& A, const FEntityId& B);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Not Equal (EntityId)", CompactNodeTitle = "!="), Category = "SpatialOS EntityId")
	static bool NotEqual_FEntityId(const FEntityId& A, const FEntityId& B);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Is Valid"), Category = "SpatialOS EntityId")
	static bool IsValid_FEntityId(const FEntityId& EntityId);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "To String"), Category = "SpatialOS EntityId")
	static FString EntityIdToString(const FEntityId& EntityId);

	UFUNCTION(BlueprintPure, Category = "SpatialOS Queries", meta = (Keywords = "construct build", NativeMakeFunc))
	static FEntityIdQueryConstraint
	MakeEntityIdQueryConstraintFromEntityId(const FEntityId& InEntityId);

	UFUNCTION(BlueprintPure, Category = "SpatialOS Queries", meta = (Keywords = "construct build", NativeMakeFunc))
	static FEntityIdQueryConstraint MakeEntityIdQueryConstraintFromInt(int InEntityId);

	UFUNCTION(BlueprintPure, Category = "SpatialOS Queries", meta = (Keywords = "construct build", NativeMakeFunc))
	static FComponentIdQueryConstraint
	MakeComponentIdQueryConstraintFromComponentId(const FComponentId& InComponentId);

	UFUNCTION(BlueprintPure, Category = "SpatialOS Queries", meta = (Keywords = "construct build", NativeMakeFunc))
	static FComponentIdQueryConstraint MakeComponentIdQueryConstraintFromInt(int InComponentId);

	UFUNCTION(BlueprintPure, Category = "SpatialOS Queries", meta = (Keywords = "construct build", NativeMakeFunc))
	static FSphereQueryConstraint MakeSphereQueryConstraint(const FVector& InPosition,
															float InRadius);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS Queries", meta = (AutoCreateRefTerm = "EntityIds, ComponentIds, Spheres", DisplayName = "SpatialOs Entity AND Query"))
	static UEntityQueryConstraint*
	SpatialOsEntityAndQuery(const TArray<FEntityIdQueryConstraint>& EntityIds,
							const TArray<FComponentIdQueryConstraint>& ComponentIds,
							const TArray<FSphereQueryConstraint>& Spheres);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS Queries", meta = (AutoCreateRefTerm = "EntityIds, ComponentIds, Spheres", DisplayName = "SpatialOs Entity OR Query"))
	static UEntityQueryConstraint*
	SpatialOsEntityOrQuery(const TArray<FEntityIdQueryConstraint>& EntityIds,
						   const TArray<FComponentIdQueryConstraint>& ComponentIds,
						   const TArray<FSphereQueryConstraint>& Spheres);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Is Command Successful"), Category = "SpatialOS Commands")
	static bool IsSuccessful_Command(const FSpatialOSCommandResult& CommandResult);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Command Status Code"), Category = "SpatialOS Commands")
	static ECommandResponseCode GetCommandStatusCode(const FSpatialOSCommandResult& CommandResult);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Command Error Message"), Category = "SpatialOS Commands")
	static FString GetCommandErrorMessage(const FSpatialOSCommandResult& CommandResult);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal (RequestId)", CompactNodeTitle = "=="), Category = "SpatialOS RequestId")
	static bool EqualEqual_FRequestId(const FRequestId& A, const FRequestId& B);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Not Equal (RequestId)", CompactNodeTitle = "!="), Category = "SpatialOS RequestId")
	static bool NotEqual_FRequestId(const FRequestId& A, const FRequestId& B);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Is RequestId Valid"), Category = "SpatialOS RequestId")
	static bool IsValid_FRequestId(const FRequestId& RequestId);
};
