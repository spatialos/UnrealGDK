// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestHandoverReplication.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Kismet/GameplayStatics.h"
#include "LoadBalancing/LayeredLBStrategy.h"

#include "DynamicReplicationHandoverCube.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialFunctionalTestStep.h"

#include "Net/UnrealNetwork.h"

UTestHandoverComponent::UTestHandoverComponent()
{
	SetIsReplicatedByDefault(true);
}

void UTestHandoverComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTestHandoverComponent, ReplicatedTestProperty);
}

AHandoverReplicationTestCube::AHandoverReplicationTestCube()
{
	bReplicates = false;

	HandoverComponent = CreateDefaultSubobject<UTestHandoverComponent>(TEXT("HandoverComponent"));
}

void AHandoverReplicationTestCube::SetTestValues(int UpdatedTestPropertyValue)
{
	HandoverTestProperty = UpdatedTestPropertyValue;
	ReplicatedTestProperty = UpdatedTestPropertyValue;
	HandoverComponent->HandoverTestProperty = UpdatedTestPropertyValue;
	HandoverComponent->ReplicatedTestProperty = UpdatedTestPropertyValue;
}

void AHandoverReplicationTestCube::RequireTestValues(ASpatialTestHandoverReplication* FunctionalTest, int RequiredValue,
													 const FString& Postfix) const
{
	FunctionalTest->RequireEqual_Int(HandoverTestProperty, RequiredValue, TEXT("Handover Cube: ") + Postfix);
	FunctionalTest->RequireEqual_Int(ReplicatedTestProperty, RequiredValue, TEXT("Replicated Cube: ") + Postfix);
	FunctionalTest->RequireEqual_Int(HandoverComponent->HandoverTestProperty, RequiredValue, TEXT("Handover Component: ") + Postfix);
	FunctionalTest->RequireEqual_Int(HandoverComponent->ReplicatedTestProperty, RequiredValue, TEXT("Replicated Component: ") + Postfix);
}

void AHandoverReplicationTestCube::OnAuthorityGained()
{
	if (ShouldResetValueToDefaultCounter == EHandoverReplicationTestStage::ChangeValuesToDefaultOnGainingAuthority)
	{
		SetTestValues(HandoverReplicationTestValues::BasicTestPropertyValue);

		ShouldResetValueToDefaultCounter = EHandoverReplicationTestStage::Final;
	}
}

void AHandoverReplicationTestCube::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHandoverReplicationTestCube, ReplicatedTestProperty);
}

/**
 *	This tests handover values replication on actors and attached components,
 *	as well as a corner case from UNR-4447: when a server sets a handover property to
 *	its default value, this property's change isn't passed to other servers.
 *
 *	The overall flow is as follows:
 *		* Spawn a cube on Server 1
 *		* Modify handover values on the cube and on an attached component
 *		* Move this cube to Server 2's authority area
 *		* Change these values to default state
 *		* Check that the value change was registered
 */

ASpatialTestHandoverReplication::ASpatialTestHandoverReplication()
	: Super()
{
	Author = TEXT("Dmitrii Kozlov");
	Description = TEXT("Test handover replication for an actor and its component");

	// Forward-Left.
	Server1Position = FVector(HandoverReplicationTestValues::WorldSize / 2, -HandoverReplicationTestValues::WorldSize / 2, 0.0f);

	// Forward-Right.
	Server2Position = FVector(HandoverReplicationTestValues::WorldSize / 2, HandoverReplicationTestValues::WorldSize / 2, 0.0f);
}

void ASpatialTestHandoverReplication::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("Check initial settings"), FWorkerDefinition::AllServers, nullptr, [this]() {
		ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(GetWorld()->GetWorldSettings());

		if (AssertIsValid(WorldSettings, TEXT("World Settings of correct type")))
		{
			AssertTrue(IsValid(WorldSettings->GetMultiWorkerSettingsClass())
						   && WorldSettings->GetMultiWorkerSettingsClass()->IsChildOf<USpatialTestHandoverReplicationMultiWorkerSettings>(),
					   TEXT("MultiWorkerSettings should be of class USpatialTestHandoverReplicationMultiWorkerSettings"));
		}
	});

	AddStep(TEXT("Server 1 spawns a HandoverCube"), FWorkerDefinition::Server(1), nullptr, [this]() {
		HandoverCube =
			GetWorld()->SpawnActor<AHandoverReplicationTestCube>(Server1Position, FRotator::ZeroRotator, FActorSpawnParameters());
		RegisterAutoDestroyActor(HandoverCube);
		FinishStep();
	});

	constexpr float StepTimeLimit = 10.0f;

	auto AddWaitingStep = [this](const FString& StepName, const FWorkerDefinition& WorkerDefinition, TFunction<void()> TickFunction) {
		AddStep(
			StepName, WorkerDefinition, nullptr, nullptr,
			[this, TickFunction](float) {
				TickFunction();
				FinishStep();
			},
			StepTimeLimit);
	};

	AddWaitingStep(TEXT("Wait until the cube is synced with all servers"), FWorkerDefinition::AllServers, [this]() {
		RequireTrue(IsValid(HandoverCube), TEXT("Server received the cube"));

		RequireHandoverCubeAuthorityAndPosition(1, Server1Position);
	});

	AddWaitingStep(TEXT("Wait until authority over the cube is given to Server 1"), FWorkerDefinition::AllServers, [this]() {
		RequireHandoverCubeAuthorityAndPosition(1, Server1Position);
	});

	AddStep(TEXT("Modify values on the Cube to non-default values"), FWorkerDefinition::Server(1), nullptr, [this]() {
		HandoverCube->SetTestValues(HandoverReplicationTestValues::UpdatedTestPropertyValue);
		HandoverCube->ShouldResetValueToDefaultCounter = EHandoverReplicationTestStage::ChangeValuesToDefaultOnGainingAuthority;
		FinishStep();
	});

	AddWaitingStep(TEXT("Wait until updated values are received on all servers"), FWorkerDefinition::AllServers, [this]() {
		HandoverCube->RequireTestValues(this, HandoverReplicationTestValues::UpdatedTestPropertyValue,
										TEXT("Non-default value received on the server"));
	});

	AddStep(TEXT("Move Cube to Server 2's authority area"), FWorkerDefinition::Server(1), nullptr, [this]() {
		HandoverCube->SetActorLocation(Server2Position);
		FinishStep();
	});

	AddWaitingStep(TEXT("Wait until authority is transferred to Server 2"), FWorkerDefinition::AllServers, [this]() {
		RequireHandoverCubeAuthorityAndPosition(2, Server2Position);
	});

	AddWaitingStep(TEXT("Wait until value is reverted to default on all servers"), FWorkerDefinition::AllServers, [this]() {
		HandoverCube->RequireTestValues(this, GetDefault<AHandoverReplicationTestCube>()->HandoverTestProperty,
										TEXT("Value reverted to default on the server"));
	});
}

void ASpatialTestHandoverReplication::RequireHandoverCubeAuthorityAndPosition(int WorkerShouldHaveAuthority,
																			  const FVector& ExpectedPosition)
{
	if (!ensureMsgf(GetLocalWorkerType() == ESpatialFunctionalTestWorkerType::Server, TEXT("Should only be called in Servers")))
	{
		return;
	}

	RequireEqual_Vector(HandoverCube->GetActorLocation(), ExpectedPosition,
						FString::Printf(TEXT("HandoverCube in %s"), *ExpectedPosition.ToCompactString()), 1.0f);

	if (WorkerShouldHaveAuthority == GetLocalWorkerId())
	{
		RequireTrue(HandoverCube->HasAuthority(), TEXT("Has Authority"));
	}
	else
	{
		RequireFalse(HandoverCube->HasAuthority(), TEXT("Doesn't Have Authority"));
	}
}

bool ASpatialTestHandoverReplication::MoveHandoverCube(const FVector& Position)
{
	if (HandoverCube->HasAuthority())
	{
		HandoverCube->SetActorLocation(Position);
		return true;
	}

	return false;
}
