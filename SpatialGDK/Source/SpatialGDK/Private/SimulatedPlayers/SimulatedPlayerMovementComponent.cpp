// Fill out your copyright notice in the Description page of Project Settings.

#include "SimulatedPlayerMovementComponent.h"

#include "GameFramework/Character.h"

void USimulatedPlayerMovementComponent::RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed)
{
	auto Owner = Cast<ACharacter>(GetOwner());
	if (Owner != nullptr)
	{
		// Just redirect move request to Owner
		Owner->AddMovementInput(MoveVelocity, 1.0f, bForceMaxSpeed);
	}
}

void USimulatedPlayerMovementComponent::RequestPathMove(const FVector& MoveInput)
{
	auto Owner = Cast<ACharacter>(GetOwner());
	if (Owner != nullptr)
	{
		// Just redirect move request to Owner
		Owner->AddMovementInput(MoveInput);
	}
}

