// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SpatialAuthorityTestGameState.generated.h"

class USpatialAuthorityTestActorComponent;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialAuthorityTestGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:	
	ASpatialAuthorityTestGameState();

	UPROPERTY()
	USpatialAuthorityTestActorComponent* AuthorityComponent;
};
