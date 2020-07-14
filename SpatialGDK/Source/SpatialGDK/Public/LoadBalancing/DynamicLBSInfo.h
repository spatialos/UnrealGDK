#pragma once

#include "SpatialCommonTypes.h"

#include "GameFramework/Info.h"
#include "Math/Box2D.h"

#include "DynamicLBSInfo.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDynamicLBStrategy, Log, All)

class USpatialNetDriver;

UCLASS(SpatialType = (Singleton, ServerOnly), Blueprintable, NotPlaceable)
class SPATIALGDK_API ADynamicLBSInfo : public AInfo
{
	GENERATED_UCLASS_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void Init(const TArray<FBox2D> WorkerCells);
	virtual void OnAuthorityGained() override;

	UPROPERTY(ReplicatedUsing = OnRep_DynamicWorkerCells)
	TArray<FBox2D> DynamicWorkerCells;

	UPROPERTY(ReplicatedUsing = OnRep_ActorCounters)
	TArray<uint32> ActorCounters;

	uint32 GetActorCounter(const VirtualWorkerId TargetWorkerId);
	void IncreseActorCounter(const VirtualWorkerId TargetWorkerId);
	void DecreaseActorCounter(const VirtualWorkerId TargetWorkerId);

private:

	UFUNCTION()
	void OnRep_DynamicWorkerCells();
	UFUNCTION()
	void OnRep_ActorCounters();

	USpatialNetDriver* NetDriver;

};
