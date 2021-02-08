// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SpatialTestAttachedComponentReplication.h"

#include "Algo/Copy.h"
#include "Algo/Count.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTestStep.h"

ASpatialTestAttachedComponentReplicationActor::ASpatialTestAttachedComponentReplicationActor()
{
	bReplicates = true;

	bAlwaysRelevant = true;
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

ASpatialTestAttachedComponentReplicationActorWithDefaultComponent::ASpatialTestAttachedComponentReplicationActorWithDefaultComponent()
{
	CreateDefaultSubobject<USpatialTestAttachedComponentReplicationComponent>(TEXT("DefaultAttachedComponent"));
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
			StepDescription, WorkerDefinition, nullptr,
			[this] {
				TimeRunningStep2 = 0.0f;
			},
			[this, TickFunction](const float DeltaTime) {
				TimeRunningStep2 += DeltaTime;
				TickFunction();
				FinishStep();
			},
			10.0f);
	};

	switch (bIsWorkingWithSceneActor)
	{
	case ESpatialTestAttachedComponentReplicationType::LevelPlaced:
		break;
	case ESpatialTestAttachedComponentReplicationType::DynamicallySpawnedWithDynamicComponent:
		AddStep(TEXT("Spawn a test actor and attach dynamic component to it"), FWorkerDefinition::Server(1), nullptr, [this] {
			auto ttttt = GetWorld()->SpawnActor<ASpatialTestAttachedComponentReplicationActor>();
			auto trterter = NewObject<USpatialTestAttachedComponentReplicationComponent>(ttttt);
			trterter->RegisterComponent();
			RegisterAutoDestroyActor(ttttt);
			FinishStep();
		});
		break;
	case ESpatialTestAttachedComponentReplicationType::DynamicallySpawnedWithDefaultComponent:
		AddStep(TEXT("Spawn a test actor with an attached default component"), FWorkerDefinition::Server(1), nullptr, [this] {
			auto ttttt = GetWorld()->SpawnActor<ASpatialTestAttachedComponentReplicationActorWithDefaultComponent>();
			RegisterAutoDestroyActor(ttttt);
			FinishStep();
		});
		break;
	default:
		checkNoEntry();
	}

	AddStep(TEXT("Setup 2"), FWorkerDefinition::Server(1), nullptr, [this] {
		FinishStep();
	});

	AddWaitingStep(TEXT("Retrieve the level placed actor"), FWorkerDefinition::AllWorkers, [this] {
		TArray<AActor*> LevelPlacedActors;
		UGameplayStatics::GetAllActorsOfClass(this, ASpatialTestAttachedComponentReplicationActor::StaticClass(), LevelPlacedActors);

		{
			// HACK: Filtering out the level placed actor.
			TArray<AActor*> Actors;
			Algo::CopyIf(LevelPlacedActors, Actors, [this](const AActor* Actor) {
				return (Actor == SceneActor) == (bIsWorkingWithSceneActor == ESpatialTestAttachedComponentReplicationType::LevelPlaced);
			});
			LevelPlacedActors = Actors;
		}

		if (LevelPlacedActors.Num() > 1)
		{
			AddError(FString::Printf(TEXT("Received %d actors while only 1 expected"), LevelPlacedActors.Num()));
		}

		if (LevelPlacedActors.Num() == 1)
		{
			LevelPlacedActor = Cast<ASpatialTestAttachedComponentReplicationActor>(LevelPlacedActors[0]);
		}
		RequireTrue(IsValid(LevelPlacedActor), TEXT("Level placed actor is valid"));

		RequireCompare_Float(TimeRunningStep2, EComparisonMethod::Greater_Than, 3.0f, TEXT("Step ran for 3 secs"));
	});

	AddWaitingStep(TEXT("Retrieve the attached component"), FWorkerDefinition::AllWorkers, [this] {
		const TSet<UActorComponent*>& AttachedComponents = LevelPlacedActor->GetComponents();

		TArray<UActorComponent*> AttachedTypedComponents;

		Algo::CopyIf(AttachedComponents, AttachedTypedComponents, [](const UActorComponent* Component) {
			return Component->IsA<USpatialTestAttachedComponentReplicationComponent>();
		});

		if (AttachedTypedComponents.Num() > 1)
		{
			AddError(FString::Printf(TEXT("Received %d components while only 1 expected"), AttachedTypedComponents.Num()));
		}

		if (AttachedTypedComponents.Num() == 1)
		{
			AttachedComponent = Cast<USpatialTestAttachedComponentReplicationComponent>(AttachedTypedComponents[0]);
		}
		RequireTrue(IsValid(AttachedComponent), TEXT("Received attached component"));

		RequireCompare_Float(TimeRunningStep2, EComparisonMethod::Greater_Than, 3.0f, TEXT("Step ran for 3 secs"));
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
