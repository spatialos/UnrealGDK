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

	bool bOnRepTestInt1Called;
	bool bOnRepTestInt2Called;
	int32 OldTestInt3;
	int32 OldTestInt4;
	TArray<int32> OldTestArray;

	UPROPERTY(ReplicatedUsing = OnRep_TestInt1)
	int32 TestInt1;

	UPROPERTY(ReplicatedUsing = OnRep_TestInt2)
	int32 TestInt2;

	UPROPERTY(ReplicatedUsing = OnRep_TestInt3)
	int32 TestInt3;

	UPROPERTY(ReplicatedUsing = OnRep_TestInt4)
	int32 TestInt4;

	UPROPERTY(ReplicatedUsing = OnRep_TestArray)
	TArray<int32> TestArray;

	UFUNCTION()
	void OnRep_TestInt1(int32 OldTestInt1);

	UFUNCTION()
	void OnRep_TestInt2(int32 OldTestInt2);

	UFUNCTION()
	void OnRep_TestInt3(int32 OldTestInt3);

	UFUNCTION()
	void OnRep_TestInt4(int32 OldTestInt4);

	UFUNCTION()
	void OnRep_TestArray(TArray<int32> OldTestArray);
};
