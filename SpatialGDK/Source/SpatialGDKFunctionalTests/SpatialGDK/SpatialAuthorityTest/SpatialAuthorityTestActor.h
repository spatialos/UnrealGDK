// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpatialAuthorityTestActor.generated.h"

class USpatialAuthorityTestActorComponent;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialAuthorityTestActor : public AActor
{
	GENERATED_BODY()

public:
	ASpatialAuthorityTestActor();

	UPROPERTY()
	USpatialAuthorityTestActorComponent* AuthorityComponent;
};
