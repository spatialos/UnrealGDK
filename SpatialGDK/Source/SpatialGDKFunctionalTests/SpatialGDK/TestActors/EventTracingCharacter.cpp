// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EventTracingCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Interop/Connection/SpatialEventTracerUserInterface.h"
#include "Net/UnrealNetwork.h"

AEventTracingCharacter::AEventTracingCharacter()
{
	bReplicates = true;

	// Increase MaxSimulationIterations to 14 so that we do not get warnings inside UCharacterMovementComponent::GetSimulationTimeStep
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	CharacterMovementComponent->MaxSimulationIterations = 14;
}

void AEventTracingCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEventTracingCharacter, TestInt);
}

void AEventTracingCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (bUseEventTracing)
	{
		if (HasAuthority())
		{
			GetWorld()->GetTimerManager().SetTimer(ToggleTestIntTimerHandle, this, &AEventTracingCharacter::ToggleTestInt, 1.0f, true);
		}
		else if(IsLocallyControlled())
		{
			GetWorld()->GetTimerManager().SetTimer(MoveCharacterTimerHandle, this, &AEventTracingCharacter::SendRPC, 1.0f, true);
		}
	}
}

void AEventTracingCharacter::ToggleTestInt()
{
	FUserSpanId SpanId = USpatialEventTracerUserInterface::TraceEvent(this, "user.send_property", "");
	USpatialEventTracerUserInterface::TraceProperty(this, this, SpanId);

	TestInt++;
}

void AEventTracingCharacter::SendRPC()
{
	FUserSpanId SpanId = USpatialEventTracerUserInterface::TraceEvent(this, "user.send_rpc", "");

	FEventTracerRPCDelegate Delegate;
	Delegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(AEventTracingCharacter, RPC));
	USpatialEventTracerUserInterface::TraceRPC(this, Delegate, SpanId);
}

void AEventTracingCharacter::RPC_Implementation()
{
	FUserSpanId SpanId;
	if (USpatialEventTracerUserInterface::GetActiveSpanId(this, SpanId))
	{
		USpatialEventTracerUserInterface::TraceEventWithCauses(this, "user.process_rpc", "", { SpanId });
	}
}

void AEventTracingCharacter::OnRepTestInt()
{
	if (!HasAuthority())
	{
		FUserSpanId CauseSpanId;
		if (USpatialEventTracerUserInterface::GetActiveSpanId(this, CauseSpanId))
		{
			USpatialEventTracerUserInterface::TraceEventWithCauses(this, "user.receive_property", "", { CauseSpanId });
		}
	}
}
