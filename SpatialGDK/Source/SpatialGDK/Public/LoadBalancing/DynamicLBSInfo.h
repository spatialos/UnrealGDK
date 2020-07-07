#pragma once

#include "SpatialCommonTypes.h"

#include "GameFramework/Info.h"
#include "Math/Box2D.h"

#include "DynamicLBSInfo.generated.h"

class USpatialNetDriver;

UCLASS(SpatialType = (Singleton, ServerOnly), Blueprintable, NotPlaceable)
class SPATIALGDK_API ADynamicLBSInfo : public AInfo
{
	GENERATED_UCLASS_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void OnAuthorityGained() override;

	UPROPERTY(ReplicatedUsing = OnRep_DynamicWorkerCells)
	TArray<FBox2D> DynamicWorkerCells;

private:

	UFUNCTION()
	void OnRep_DynamicWorkerCells();

	USpatialNetDriver* NetDriver;

};
