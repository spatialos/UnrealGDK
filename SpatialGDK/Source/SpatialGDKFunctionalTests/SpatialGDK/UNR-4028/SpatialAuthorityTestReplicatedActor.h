// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialAuthorityTestActor.h"
#include "SpatialAuthorityTestReplicatedActor.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialAuthorityTestReplicatedActor : public ASpatialAuthorityTestActor
{
	GENERATED_BODY()
	
public:	
	ASpatialAuthorityTestReplicatedActor();

	virtual void OnAuthorityGained() override;
};
