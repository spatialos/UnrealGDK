// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SpatialTestAttachedComponentReplication.h"

#include "Algo/Copy.h"
#include "Algo/Count.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTestStep.h"

struct FSpatialTestComponentReplicationValues
{
};

ASpatialTestAttachedComponentReplicationActor::ASpatialTestAttachedComponentReplicationActor()
{
	bReplicates = true;
}

USpatialTestAttachedComponentReplicationComponent::USpatialTestAttachedComponentReplicationComponent()
{
	SetIsReplicatedByDefault(true);
}

void USpatialTestAttachedComponentReplicationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ReplicatedValue);
}

ASpatialTestAttachedComponentReplication::ASpatialTestAttachedComponentReplication()
{
	Author = TEXT("Dmitrii Kozlov <dmitriikozlov@improbable.io>");
	Description =
		TEXT("Check different types of attached component replication:" LINE_TERMINATOR "* Component present on a C++ actor" LINE_TERMINATOR
			 "* Component present on a BP actor" LINE_TERMINATOR "* Component dynamically added to a C++ actor" LINE_TERMINATOR
			 "* Component dynamically added to a BP actor" LINE_TERMINATOR "* Component added in a map to a C++ actor" LINE_TERMINATOR
			 "* Component added in a map to a BP actor" LINE_TERMINATOR);
}

void ASpatialTestAttachedComponentReplication::PrepareTest()
{
	Super::PrepareTest();

	auto AddWaitingStep = [this](const FString& StepDescription, const FWorkerDefinition& WorkerDefinition,
								 TFunction<void()> TickFunction) {
		AddStep(
			StepDescription, WorkerDefinition, nullptr, nullptr,
			[this, TickFunction](float) {
				TickFunction();
				FinishStep();
			},
			10.0f);
	};

	AddWaitingStep(TEXT("Retrieve the level placed actor"), FWorkerDefinition::AllWorkers, [this] {
		TArray<AActor*> LevelPlacedActors;
		UGameplayStatics::GetAllActorsOfClass(this, ASpatialTestAttachedComponentReplicationActor::StaticClass(), LevelPlacedActors);
		RequireEqual_Int(LevelPlacedActors.Num(), 1, TEXT("Received one level placed actor"));
		AssertTrue(LevelPlacedActors.Num() <= 1, TEXT("No more than one level placed actors are allowed"));
		if (LevelPlacedActors.Num() == 1)
		{
			LevelPlacedActor = Cast<ASpatialTestAttachedComponentReplicationActor>(LevelPlacedActors[0]);
		}
		RequireTrue(IsValid(LevelPlacedActor), TEXT("Level placed actor is valid"));

		RequireCompare_Float(TimeRunningStep, EComparisonMethod::Greater_Than, 3.0f, TEXT("Step ran for 3 secs"));
	});

	AddWaitingStep(TEXT("Retrieve the attached component"), FWorkerDefinition::AllWorkers, [this] {
		const TSet<UActorComponent*>& AttachedComponents = LevelPlacedActor->GetComponents();

		TArray<UActorComponent*> AttachedTypedComponents;

		Algo::CopyIf(AttachedComponents, AttachedTypedComponents, [](const UActorComponent* Component) {
			return Component->IsA<USpatialTestAttachedComponentReplicationComponent>();
		});

		RequireEqual_Int(AttachedTypedComponents.Num(), 1, TEXT("One attached component on the actor"));
		AssertTrue(AttachedTypedComponents.Num() <= 1, TEXT("No more than one attached components are allowed"));

		if (AttachedTypedComponents.Num() == 1)
		{
			AttachedComponent = Cast<USpatialTestAttachedComponentReplicationComponent>(AttachedTypedComponents[0]);
		}

		UE_LOG(LogTemp, Log, TEXT("%s %d -> %f"),
			   GetLocalWorkerType() == ESpatialFunctionalTestWorkerType::Server ? TEXT("Server") : TEXT("Client"), GetLocalWorkerId(),
			   TimeRunningStep);

		RequireCompare_Float(TimeRunningStep, EComparisonMethod::Greater_Than, 3.0f, TEXT("Step ran for 3 secs"));
	});

	AddStep(TEXT("Modify replicated value on the component"), FWorkerDefinition::Server(1), nullptr, [this] {
		AssertTrue(LevelPlacedActor->HasAuthority(), TEXT("Server 1 has authority over the actor"));
		AttachedComponent->ReplicatedValue = SpatialTestAttachedComponentReplicationValues::ChangedValue;
		FinishStep();
	});

	AddWaitingStep(TEXT("Check that the updated value is received"), FWorkerDefinition::AllWorkers, [this] {
		RequireEqual_Int(AttachedComponent->ReplicatedValue, SpatialTestAttachedComponentReplicationValues::ChangedValue,
						 TEXT("Updated value received"));
	});
}
