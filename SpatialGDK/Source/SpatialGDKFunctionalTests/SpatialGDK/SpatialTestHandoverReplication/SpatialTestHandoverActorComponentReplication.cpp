// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestHandoverActorComponentReplication.h"
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

	DOREPLIFETIME(ThisClass, ReplicatedTestProperty);
}

AHandoverReplicationTestCube::AHandoverReplicationTestCube()
{
	bReplicates = true;

	bAlwaysRelevant = true;

	HandoverComponent = CreateDefaultSubobject<UTestHandoverComponent>(TEXT("HandoverComponent"));
}

void AHandoverReplicationTestCube::SetTestValues(int UpdatedTestPropertyValue)
{
	HandoverTestProperty = UpdatedTestPropertyValue;
	ReplicatedTestProperty = UpdatedTestPropertyValue;
	HandoverComponent->HandoverTestProperty = UpdatedTestPropertyValue;
	HandoverComponent->ReplicatedTestProperty = UpdatedTestPropertyValue;
}

void AHandoverReplicationTestCube::RequireTestValues(ASpatialTestHandoverActorComponentReplication* FunctionalTest, int RequiredValue,
													 const FString& Postfix) const
{
	FunctionalTest->RequireEqual_Int(HandoverTestProperty, RequiredValue, TEXT("Handover Cube: ") + Postfix);
	FunctionalTest->RequireEqual_Int(ReplicatedTestProperty, RequiredValue, TEXT("Replicated Cube: ") + Postfix);
	FunctionalTest->RequireEqual_Int(HandoverComponent->HandoverTestProperty, RequiredValue, TEXT("Handover Component: ") + Postfix);
	FunctionalTest->RequireEqual_Int(HandoverComponent->ReplicatedTestProperty, RequiredValue, TEXT("Replicated Component: ") + Postfix);
}

void AHandoverReplicationTestCube::OnAuthorityGained()
{
	if (TestStage == EHandoverReplicationTestStage::ChangeValuesToDefaultOnGainingAuthority)
	{
		SetTestValues(HandoverReplicationTestValues::BasicTestPropertyValue);

		TestStage = EHandoverReplicationTestStage::Final;
	}
}

void AHandoverReplicationTestCube::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ReplicatedTestProperty);
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

ASpatialTestHandoverActorComponentReplication::ASpatialTestHandoverActorComponentReplication()
	: Super()
{
	Author = TEXT("Dmitrii Kozlov");
	Description = TEXT("Test handover replication for an actor and its component");

	// Forward-Left, will be in Server 1's authority area.
	Server1Position = FVector(HandoverReplicationTestValues::WorldSize / 4, -HandoverReplicationTestValues::WorldSize / 4, 0.0f);

	// Forward-Right, will be in Server 2's authority area.
	Server2Position = FVector(HandoverReplicationTestValues::WorldSize / 4, HandoverReplicationTestValues::WorldSize / 4, 0.0f);

	bReplicates = true;
}

void ASpatialTestHandoverActorComponentReplication::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, HandoverCube);
}

void ASpatialTestHandoverActorComponentReplication::PrepareTest()
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

		FinishStep();
	});

	AddStep(TEXT("Server 1 spawns a HandoverCube"), FWorkerDefinition::Server(1), nullptr, [this]() {
		HandoverCube =
			GetWorld()->SpawnActor<AHandoverReplicationTestCube>(Server1Position, FRotator::ZeroRotator, FActorSpawnParameters());
		SaveHandoverCube(HandoverCube);
		RegisterAutoDestroyActor(HandoverCube);
		FinishStep();
	});

	auto AddWaitingStep = [this](const FString& StepName, const FWorkerDefinition& WorkerDefinition, TFunction<void()> TickFunction) {
		constexpr float StepTimeLimit = 10.0f;

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
	});

	AddStep(TEXT("Modify values on the Cube to non-default values"), FWorkerDefinition::Server(1), nullptr, [this]() {
		HandoverCube->SetTestValues(HandoverReplicationTestValues::UpdatedTestPropertyValue);
		HandoverCube->TestStage = EHandoverReplicationTestStage::ChangeValuesToDefaultOnGainingAuthority;
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

void ASpatialTestHandoverActorComponentReplication::SaveHandoverCube_Implementation(AHandoverReplicationTestCube* InHandoverCube)
{
	HandoverCube = InHandoverCube;
}

void ASpatialTestHandoverActorComponentReplication::RequireHandoverCubeAuthorityAndPosition(int WorkerShouldHaveAuthority,
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

bool ASpatialTestHandoverActorComponentReplication::MoveHandoverCube(const FVector& Position)
{
	if (HandoverCube->HasAuthority())
	{
		HandoverCube->SetActorLocation(Position);
		return true;
	}

	return false;
}
