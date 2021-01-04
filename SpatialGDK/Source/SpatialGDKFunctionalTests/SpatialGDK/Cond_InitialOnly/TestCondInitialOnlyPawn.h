// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "TestCondInitialOnlyPawn.generated.h"

class UCameraComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTestCondInitialOnlyPawn, Log, All);

UCLASS()
class ATestCondInitialOnlyPawn : public APawn
{
	GENERATED_BODY()
private:
	UPROPERTY(Replicated)
	int32 TestValue;

	UPROPERTY()
	UStaticMeshComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere, Category = "Spatial Functional Test")
	UCameraComponent* CameraComponent;

public:
	ATestCondInitialOnlyPawn();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetTestValue(int32 Value);

	int32 GetTestValue();
};
