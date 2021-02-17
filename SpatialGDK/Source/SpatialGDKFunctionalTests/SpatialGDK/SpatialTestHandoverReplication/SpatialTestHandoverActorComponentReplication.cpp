// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestHandoverActorComponentReplication.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Kismet/GameplayStatics.h"
#include "LoadBalancing/LayeredLBStrategy.h"

#include "DynamicReplicationHandoverCube.h"
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
	FunctionalTest->RequireEqual_Int(HandoverTestProperty, RequiredValue,
									 FString::Printf(TEXT("Handover Cube = %d: %s"), RequiredValue, *Postfix));
	FunctionalTest->RequireEqual_Int(ReplicatedTestProperty, RequiredValue,
									 FString::Printf(TEXT("Replicated Cube = %d: %s"), RequiredValue, *Postfix));
	FunctionalTest->RequireEqual_Int(HandoverComponent->HandoverTestProperty, RequiredValue,
									 FString::Printf(TEXT("Handover Component = %d: %s"), RequiredValue, *Postfix));
	FunctionalTest->RequireEqual_Int(HandoverComponent->ReplicatedTestProperty, RequiredValue,
									 FString::Printf(TEXT("Replicated Component = %d: %s"), RequiredValue, *Postfix));
}

void AHandoverReplicationTestCube::OnAuthorityGained()
{
	Super::OnAuthorityGained();

	if (TestStage == EHandoverReplicationTestStage::ChangeValuesToDefaultOnGainingAuthority)
	{
		SetTestValues(HandoverReplicationTestValues::BasicTestPropertyValue);

		TestStage = EHandoverReplicationTestStage::Final;

		SetActorLocation(HandoverReplicationTestValues::Server1Position);
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

	bReplicates = true;
}

void ASpatialTestHandoverActorComponentReplication::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("Server 1 spawns a HandoverCube"), FWorkerDefinition::Server(1), nullptr, [this]() {
		HandoverCube = GetWorld()->SpawnActor<AHandoverReplicationTestCube>(HandoverReplicationTestValues::Server1Position,
																			FRotator::ZeroRotator, FActorSpawnParameters());
		RegisterAutoDestroyActor(HandoverCube);
		FinishStep();
	});

	// A helper to add awaiting steps to the test; RequireXXX used in TickFunction
	// must be satisfied before StepTimeLimit is over, or the step would fail.
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
		TArray<AActor*> DiscoveredReplicationCubes;
		UGameplayStatics::GetAllActorsOfClass(this, AHandoverReplicationTestCube::StaticClass(), DiscoveredReplicationCubes);
		RequireEqual_Int(DiscoveredReplicationCubes.Num(), 1, TEXT("Cube discovered"));
		if (DiscoveredReplicationCubes.Num() == 1)
		{
			HandoverCube = Cast<AHandoverReplicationTestCube>(DiscoveredReplicationCubes[0]);
		}
		RequireTrue(IsValid(HandoverCube), TEXT("Server received the cube"));
	});

	AddStep(TEXT("Modify values on the Cube to non-default values"), FWorkerDefinition::Server(1), nullptr, [this]() {
		HandoverCube->SetTestValues(HandoverReplicationTestValues::UpdatedTestPropertyValue);
		// This causes the cube to update its values on OnAuthorityGained, so once another server gains authority,
		// it will update the values before any other code had an opportunity to read them.
		HandoverCube->TestStage = EHandoverReplicationTestStage::ChangeValuesToDefaultOnGainingAuthority;
		FinishStep();
	});

	AddWaitingStep(TEXT("Wait until updated values are received on all servers"), FWorkerDefinition::AllServers, [this]() {
		HandoverCube->RequireTestValues(this, HandoverReplicationTestValues::UpdatedTestPropertyValue,
										TEXT("Non-default value received on the server"));
		RequireTrue(HandoverCube->TestStage == EHandoverReplicationTestStage::ChangeValuesToDefaultOnGainingAuthority,
					TEXT("Cube state received on the server"));
	});

	// This step will trigger value change to the default one, after which the cube would be transported back to Server 1's authority area
	AddStep(TEXT("Move Cube to Server 2's authority area"), FWorkerDefinition::Server(1), nullptr, [this]() {
		HandoverCube->SetActorLocation(HandoverReplicationTestValues::Server2Position);
		FinishStep();
	});

	AddWaitingStep(TEXT("Wait until value is reverted to default on all servers"), FWorkerDefinition::AllServers, [this]() {
		HandoverCube->RequireTestValues(this, GetDefault<AHandoverReplicationTestCube>()->HandoverTestProperty,
										TEXT("Value reverted to default on the server"));
	});
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
