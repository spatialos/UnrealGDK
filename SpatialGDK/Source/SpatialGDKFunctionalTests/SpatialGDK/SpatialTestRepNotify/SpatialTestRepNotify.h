// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestRepNotify.generated.h"

class ASpatialTestRepNotifyActor;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestRepNotify : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestRepNotify();

	virtual void PrepareTest() override;

	UPROPERTY(Replicated)
	ASpatialTestRepNotifyActor* TestActor = nullptr;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
};
