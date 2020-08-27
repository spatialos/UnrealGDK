// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SpatialAuthorityTestGameMode.generated.h"

class USpatialAuthorityTestActorComponent;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialAuthorityTestGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:	
	ASpatialAuthorityTestGameMode();

	UPROPERTY()
	USpatialAuthorityTestActorComponent* AuthorityComponent;
};
