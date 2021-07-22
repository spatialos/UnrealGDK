// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "SpatialTestNetReceive.generated.h"

class ASpatialTestNetReceiveActor;
class USpatialTestNetReceiveSubobject;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestNetReceive : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestNetReceive();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	virtual void PrepareTest() override;

	UPROPERTY(Replicated)
	ASpatialTestNetReceiveActor* TestActor;

	FVector Server1Pos = FVector(100, -100, 300);
};

UCLASS()
class ASpatialTestNetReceiveActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	ASpatialTestNetReceiveActor();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	USpatialTestNetReceiveSubobject* Subobject;
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialTestNetReceiveSubobject : public UActorComponent
{
	GENERATED_BODY()

public:
	USpatialTestNetReceiveSubobject();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void PreNetReceive() override;
	void PostNetReceive() override;

	bool bOnRepTestInt1Called;

	bool bPreNetReceiveCalled;
	bool bPostNetReceiveCalled;

	UPROPERTY(ReplicatedUsing = OnRep_TestInt1)
	int32 TestInt;

	UPROPERTY(Replicated)
	int32 ServerOnlyTestInt;

	bool bPostNetCalledBeforePreNet;
	bool bRepNotifyCalledBeforePreNet;

	bool bPreNetCalledBeforePostNet;
	bool bRepNotifyCalledBeforePostNet;

	bool bPreNetCalledBeforeRepNotify;
	bool bPostNetCalledBeforeRepNotify;

	int32 PreNetNumTimesCalled;
	int32 PostNetNumTimesCalled;
	int32 RepNotifyNumTimesCalled;

	UFUNCTION()
	void OnRep_TestInt1(int32 OldTestInt1);
};
