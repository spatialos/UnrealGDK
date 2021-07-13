// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TestMovementCharacter.generated.h"

UCLASS()
class ATestMovementCharacter : public ACharacter
{
	GENERATED_BODY()

private:
	UPROPERTY()
	UStaticMeshComponent* SphereComponent;

	UPROPERTY()
	class UCameraComponent* CameraComponent;

public:
	ATestMovementCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	FVector PreviousLocation;

	float Speed;

	UFUNCTION(Client, Reliable)
	void UpdateCameraLocationAndRotation(FVector Location, FRotator NewRotation);
};
