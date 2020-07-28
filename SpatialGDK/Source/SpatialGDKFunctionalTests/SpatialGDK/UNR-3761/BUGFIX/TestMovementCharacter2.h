// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TestMovementCharacter2.generated.h"

UCLASS()
class ATestMovementCharacter2 : public ACharacter
{
	GENERATED_BODY()

private:
	UPROPERTY()
	UStaticMeshComponent* SphereComponent;

	UPROPERTY()
	class UCameraComponent* CameraComponent;

public:
	ATestMovementCharacter2();

	UFUNCTION(Client, Reliable)
	void UpdateCameraLocationAndRotation(FVector Location, FRotator NewRotation);
};
