// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "TestPossessionPawn.generated.h"

class UCameraComponent;

UCLASS()
class ATestPossessionPawn : public APawn
{
	GENERATED_BODY()
private:
	UPROPERTY()
	UStaticMeshComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere, Category = "Spatial Functional Test")
	UCameraComponent* CameraComponent;

public:
	ATestPossessionPawn();
};
