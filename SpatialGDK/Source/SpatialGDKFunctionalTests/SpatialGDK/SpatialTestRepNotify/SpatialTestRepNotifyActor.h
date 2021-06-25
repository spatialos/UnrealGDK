// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"

#include "SpatialTestRepNotifyActor.generated.h"

class USpatialTestRepNotifySubobject;
UCLASS()
class ASpatialTestRepNotifyActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	ASpatialTestRepNotifyActor();
	void SetUpTestProperties();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

	UPROPERTY(Replicated)
	USpatialTestRepNotifySubobject* TestSubobject = nullptr;
	int32 ExpectedSubobjectIntProperty = 0;
	bool bSubobjectIntPropertyWasExpectedProperty;

	UFUNCTION()
	void OnRep_OnChangedRepNotifyInt1(int32 OldOnChangedRepNotifyInt1);

	UFUNCTION()
	void OnRep_AlwaysRepNotifyInt1(int32 OldAlwaysRepNotifyInt1);

	UFUNCTION()
	void OnRep_OnChangedRepNotifyInt2(int32 InOldOnChangedRepNotifyInt2);

	UFUNCTION()
	void OnRep_AlwaysRepNotifyInt2(int32 InOldAlwaysRepNotifyInt2);

	UFUNCTION()
	void OnRep_TestArray(TArray<int32> InOldTestArray);
};
