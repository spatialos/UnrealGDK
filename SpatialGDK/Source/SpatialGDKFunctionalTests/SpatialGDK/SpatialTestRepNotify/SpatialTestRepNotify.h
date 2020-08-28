// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestRepNotify.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestRepNotify : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestRepNotify();

	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool bOnRepOnChangedRepNotifyInt1Called;

	bool bOnRepAlwaysRepNotifyInt1Called;

	int32 OldOnChangedRepNotifyInt2;

	int32 OldAlwaysRepNotifyInt2;

	TArray<int32> OldTestArray;

	UPROPERTY(ReplicatedUsing = OnRep_OnChangedRepNotifyInt1)
	int32 OnChangedRepNotifyInt1;

	UPROPERTY(ReplicatedUsing = OnRep_AlwaysRepNotifyInt1)
	int32 AlwaysRepNotifyInt1;

	UPROPERTY(ReplicatedUsing = OnRep_OnChangedRepNotifyInt2)
	int32 OnChangedRepNotifyInt2;

	UPROPERTY(ReplicatedUsing = OnRep_AlwaysRepNotifyInt2)
	int32 AlwaysRepNotifyInt2;

	UPROPERTY(ReplicatedUsing = OnRep_TestArray)
	TArray<int32> TestArray;

	UFUNCTION()
	void OnRep_OnChangedRepNotifyInt1(int32 OldOnChangedRepNotifyInt1);

	UFUNCTION()
	void OnRep_AlwaysRepNotifyInt1(int32 OldAlwaysRepNotifyInt1);

	UFUNCTION()
	void OnRep_OnChangedRepNotifyInt2(int32 OldOnChangedRepNotifyInt2);

	UFUNCTION()
	void OnRep_AlwaysRepNotifyInt2(int32 OldAlwaysRepNotifyInt2);

	UFUNCTION()
	void OnRep_TestArray(TArray<int32> OldTestArray);
};
