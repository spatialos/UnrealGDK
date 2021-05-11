// NWX_BEGIN - https://improbableio.atlassian.net/browse/NWX-18915 - [IMPROVEMENT] ClosestNInterestComponent
// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "EngineClasses/Components/AbstractInterestComponent.h"
#include "Schema/Interest.h"
#include "ClosestNInterestComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogClosestNInterestComponent, Log, All);

UCLASS(ClassGroup = (SpatialGDK), SpatialType = ServerOnly, Meta = (BlueprintSpawnableComponent))
class SPATIALGDK_API UClosestNInterestComponent : public UAbstractInterestComponent
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Interest", meta = (ToolTip = "Target class to express Interest in."))
	TSubclassOf<AActor> TargetActorClass;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Interest", meta = (ToolTip = "Express Interest in at most this many target actors, regardless of how many are on the UnrealServer."))
	int32 TargetActorMaxCount = 20;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Interest", meta = (ToolTip = "Frequency UnrealServer should recalculate Interest queries and send them to SpatialOS Runtime (Hz)."))
	float UnrealServerUpdateFrequency = 0.2f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Interest", meta = (ToolTip = "Frequency SpatialOS Runtime should run the query and send the result to the client (Hz)."))
	float RuntimeUpdateFrequency = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Interest", meta = (ToolTip = "Maximum range that a target actor can be away from the owning client's PlayerController or currently possessed Pawn and still be considered for Interest (cm)."))
	float MaxInterestRange = 15000.0f;
	float MaxInterestRangeSqr;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Interest", meta = (ToolTip = "For each target actor we will express Interest in, also express interest in it's attachment hierarchy."))
	bool bIncludeHierarchy = false;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Advanced", meta = (ToolTip = "Reduce dynamic memory allocations from this component by setting this to the max expected # of target actors on the UnrealServer."))
	int32 SoftMaxTargetActorsOnServerCount = 16;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Advanced", meta = (EditCondition="bIncludeHierarchy", ToolTip = "Reduce dynamic memory allocations from this component by setting this to the max expected # of replicating attached actors in the target actor's hierarchy on the UnrealServer."))
	int32 SoftMaxAttachedActorsPerTargetActorCount = 24;

	// ISpatialInterestProvider
	virtual void PopulateFrequencyToConstraintsMap(const USpatialClassInfoManager& ClassInfoManager,
		SpatialGDK::FrequencyToConstraintsMap& OutFrequencyToQueryConstraints) const override;

	UFUNCTION(Server, Unreliable, BlueprintCallable, Category = "Advanced")
	void UpdateInterestRange(float Value);

private:
	void UpdateQuery();

	void FindTargetActors();
	void FindTargetEntityIds();

	void ConstructQuery();

	FTimerHandle UpdateQueryTimerHandle;
	TArray<SpatialGDK::QueryConstraint> ConstraintList;

	TArray<AActor*> TargetActors;

	struct FActorSortData
	{
		TWeakObjectPtr<AActor> Actor;
		float DistanceSqr;
	};

	TArray<FActorSortData> TargetActorsSorted;

	TArray<int64> TargetEntityIds;
	TArray<int64> PreviousTargetEntityIds;

	// Constants per runtime invocation, need to be calculated at runtime from parameters set on the component
	int32 SoftMaxEntityCount = 0; 

	static bool bSoftTargetActorLimitExceededWarningShown;
	static bool bSoftTargetEntityLimitExceededWarningShown;
};
