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

	auto AddGenericTestCases = [this, &AddWaitingStep](TFunction<void()> Setup) {
		AddStep(TEXT("Setup"), FWorkerDefinition::AllWorkers, nullptr, [this, Setup] {
			LevelPlacedActor = nullptr;
			AttachedComponent = nullptr;

			FinishStep();
		});

		AddStep(TEXT("Setup 2"), FWorkerDefinition::Server(1), nullptr, [Setup, this] {
			Setup();
			FinishStep();
		});

		AddWaitingStep(TEXT("Retrieve the level placed actor"), FWorkerDefinition::AllWorkers, [this] {
			TArray<AActor*> LevelPlacedActors;
			UGameplayStatics::GetAllActorsOfClass(this, ASpatialTestAttachedComponentReplicationActor::StaticClass(), LevelPlacedActors);
			TArray<AActor*> Actors;
			Algo::CopyIf(LevelPlacedActors, Actors, [this](const AActor* Actor) {
				return (Actor == SceneActor) == (bIsWorkingWithSceneActor == ESpatialTestAttachedComponentReplicationType::LevelPlaced);
			});
			// RequireEqual_Int(LevelPlacedActors.Num(), 1, TEXT("Received one level placed actor"));
			// AssertTrue(LevelPlacedActors.Num() <= 1, TEXT("No more than one level placed actors are allowed"));
			LevelPlacedActors = Actors;
			if (LevelPlacedActors.Num() > 1)
			{
				AddError(FString::Printf(TEXT("Received %d actors while only 1 expected"), LevelPlacedActors.Num()));
			}
			auto ttt = LevelPlacedActors.FindByPredicate([this](const AActor* Actor) {
				return (Actor == SceneActor) == (bIsWorkingWithSceneActor == ESpatialTestAttachedComponentReplicationType::LevelPlaced);
			});
			if (ttt != nullptr)
			{
				LevelPlacedActor = Cast<ASpatialTestAttachedComponentReplicationActor>(*ttt);
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

			RequireEqual_Int(AttachedTypedComponents.Num(), 1, TEXT("One attached component on the actor"));
			if (AttachedTypedComponents.Num() > 1)
			{
				LogStep(ELogVerbosity::Error,
						FString::Printf(TEXT("Received %d components while only 1 expected"), AttachedTypedComponents.Num()));
			}

			if (AttachedTypedComponents.Num() == 1)
			{
				AttachedComponent = Cast<USpatialTestAttachedComponentReplicationComponent>(AttachedTypedComponents[0]);
			}

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
	};

	AddGenericTestCases([this] {
		switch (bIsWorkingWithSceneActor)
		{
		case ESpatialTestAttachedComponentReplicationType::LevelPlaced:
			break;
		case ESpatialTestAttachedComponentReplicationType::DynamicallySpawnedWithDynamicComponent:
		{
			auto ttttt = GetWorld()->SpawnActor<ASpatialTestAttachedComponentReplicationActor>();
			auto trterter = NewObject<USpatialTestAttachedComponentReplicationComponent>(ttttt);
			trterter->RegisterComponent();
			RegisterAutoDestroyActor(ttttt);
		}
		break;
		case ESpatialTestAttachedComponentReplicationType::DynamicallySpawnedWithDefaultComponent:
		{
			auto ttttt = GetWorld()->SpawnActor<ASpatialTestAttachedComponentReplicationActorWithDefaultComponent>();
			RegisterAutoDestroyActor(ttttt);
		}
		break;
		default:
			checkNoEntry();
		}
	});
}
