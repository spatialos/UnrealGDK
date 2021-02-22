// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SpatialTestAttachedComponentReplication.h"

#include "Algo/Copy.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTestStep.h"

void USpatialTestAttachedComponentReplicationTestMap::CreateCustomContentForMap()
{
	Super::CreateCustomContentForMap();

	ULevel* TestLevel = World->GetCurrentLevel();

	AActor* LevelActor = AddActorToLevel<ASpatialTestAttachedComponentReplicationActorForLevelPlacing>(TestLevel, FTransform::Identity);

	// Adding an instance component...
	UActorComponent* InstanceComponent =
		NewObject<USpatialTestAttachedComponentReplicationComponent>(LevelActor, TEXT("PlacedActorComponent"), RF_Transactional);
	LevelActor->AddInstanceComponent(InstanceComponent);
	InstanceComponent->OnComponentCreated();
	InstanceComponent->RegisterComponent();

	auto AddTest = [this, TestLevel](ESpatialTestAttachedComponentReplicationType TestType) {
		ASpatialTestAttachedComponentReplication* Test =
			AddActorToLevel<ASpatialTestAttachedComponentReplication>(TestLevel, FTransform::Identity);
		Test->AssignedTestType = TestType;
	};

	AddTest(ESpatialTestAttachedComponentReplicationType::LevelPlaced);
	AddTest(ESpatialTestAttachedComponentReplicationType::DynamicallySpawnedWithDynamicComponent);
	AddTest(ESpatialTestAttachedComponentReplicationType::DynamicallySpawnedWithDefaultComponent);
}

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
		TEXT("Check different types of attached component replication:") LINE_TERMINATOR TEXT("* Component added in a map to an actor")
			LINE_TERMINATOR TEXT("* Component present on an actor") LINE_TERMINATOR TEXT("* Component dynamically added to an actor");
}

struct FSpatialTestAttachedComponentReplicationTypeDescription
{
	FString Description;
	TSubclassOf<ASpatialTestAttachedComponentReplicationActor> TestActorClass;
};

static FSpatialTestAttachedComponentReplicationTypeDescription GetTestTypeDescription(
	const ESpatialTestAttachedComponentReplicationType TestType)
{
	switch (TestType)
	{
	case ESpatialTestAttachedComponentReplicationType::LevelPlaced:
		return { TEXT("Level Placed Actor"), ASpatialTestAttachedComponentReplicationActorForLevelPlacing::StaticClass() };
	case ESpatialTestAttachedComponentReplicationType::DynamicallySpawnedWithDynamicComponent:
		return { TEXT("Dynamic Actor with Dynamic Component"),
				 ASpatialTestAttachedComponentReplicationActorWithDynamicComponent::StaticClass() };
	case ESpatialTestAttachedComponentReplicationType::DynamicallySpawnedWithDefaultComponent:
		return { TEXT("Dynamic Actor with Default Component"),
				 ASpatialTestAttachedComponentReplicationActorWithDefaultComponent::StaticClass() };
	default:
		checkNoEntry();
	}

	return {};
}

static UClass* GetClassForTestType(const ESpatialTestAttachedComponentReplicationType TestType)
{
	return GetTestTypeDescription(TestType).TestActorClass;
}

static FString DescribeTestType(const ESpatialTestAttachedComponentReplicationType TestType)
{
	return GetTestTypeDescription(TestType).Description + TEXT(": ");
}

void ASpatialTestAttachedComponentReplication::PrepareTest()
{
	Super::PrepareTest();

	GenerateTestSteps(AssignedTestType);
}

/*
 * This test checks component replication behavior:
 * * An instance component on a level-placed actor
 * * A component that is added dynamically to a dynamically spawned actor
 * * A default component of a dynamically spawned actor
 * In all of these cases, the expectation is that there is a single component
 * on both clients and servers.
 */
void ASpatialTestAttachedComponentReplication::GenerateTestSteps(ESpatialTestAttachedComponentReplicationType TestType)
{
	// A helper to add awaiting steps to the test; RequireXXX used in TickFunction
	// must be satisfied before StepTimeLimit is over, or the step would fail.
	// Is also controls TimeInStep so we can track time elapsed in a given step inside of TickFunction
	auto AddWaitingStep = [this](const FString& StepDescription, const FWorkerDefinition& WorkerDefinition,
								 TFunction<void()> TickFunction) {
		AddStep(
			StepDescription, WorkerDefinition, nullptr,
			[this] {
				TimeInStep = 0.0f;
			},
			[this, TickFunction](const float DeltaTime) {
				TimeInStep += DeltaTime;
				TickFunction();
				FinishStep();
			},
			10.0f);
	};

	AddStep(DescribeTestType(TestType) + TEXT("Clean up before running test"), FWorkerDefinition::AllWorkers, nullptr, [this] {
		LevelPlacedActor = nullptr;
		AttachedComponent = nullptr;

		DeleteActorsRegisteredForAutoDestroy();

		FinishStep();
	});

	switch (TestType)
	{
	case ESpatialTestAttachedComponentReplicationType::LevelPlaced:
		// Intentionally empty; this actor should be present inside of the map.
		break;
	case ESpatialTestAttachedComponentReplicationType::DynamicallySpawnedWithDynamicComponent:
		AddStep(TEXT("Spawn a test actor and attach dynamic component to it"), FWorkerDefinition::Server(1), nullptr, [this] {
			AActor* ActorWithDynamicComponent = GetWorld()->SpawnActor<ASpatialTestAttachedComponentReplicationActorWithDynamicComponent>();
			UActorComponent* DynamicComponent = NewObject<USpatialTestAttachedComponentReplicationComponent>(ActorWithDynamicComponent);
			DynamicComponent->RegisterComponent();
			RegisterAutoDestroyActor(ActorWithDynamicComponent);
			FinishStep();
		});
		break;
	case ESpatialTestAttachedComponentReplicationType::DynamicallySpawnedWithDefaultComponent:
		AddStep(TEXT("Spawn a test actor with an attached default component"), FWorkerDefinition::Server(1), nullptr, [this] {
			AActor* TestActorWithDefaultComponent =
				GetWorld()->SpawnActor<ASpatialTestAttachedComponentReplicationActorWithDefaultComponent>();
			RegisterAutoDestroyActor(TestActorWithDefaultComponent);
			FinishStep();
		});
		break;
	default:
		checkNoEntry();
	}

	AddWaitingStep(DescribeTestType(TestType) + TEXT("Retrieve the level placed actor"), FWorkerDefinition::AllWorkers, [this, TestType] {
		UClass* ActorClassToLookFor = GetClassForTestType(TestType);

		TArray<AActor*> LevelPlacedActors;
		UGameplayStatics::GetAllActorsOfClass(this, ActorClassToLookFor, LevelPlacedActors);

		if (LevelPlacedActors.Num() > 1)
		{
			AddError(FString::Printf(TEXT("Received %d actors while only 1 expected"), LevelPlacedActors.Num()));
		}
		else if (LevelPlacedActors.Num() == 1)
		{
			LevelPlacedActor = Cast<ASpatialTestAttachedComponentReplicationActor>(LevelPlacedActors[0]);
		}
		RequireTrue(IsValid(LevelPlacedActor), TEXT("Discovered actor is valid"));

		constexpr float ActorPollDuration = 3.0f;

		// We need to run this step for some time to catch test conditions failing at a later date,
		// for example, two test actors appearing on clients.
		RequireCompare_Float(TimeInStep, EComparisonMethod::Greater_Than, ActorPollDuration,
							 FString::Printf(TEXT("Step ran for %f secs"), ActorPollDuration));
	});

	AddWaitingStep(DescribeTestType(TestType) + TEXT("Retrieve the attached component"), FWorkerDefinition::AllWorkers, [this] {
		TArray<UActorComponent*> AttachedComponents;

		Algo::CopyIf(LevelPlacedActor->GetComponents(), AttachedComponents, [](const UActorComponent* Component) {
			return Component->IsA<USpatialTestAttachedComponentReplicationComponent>();
		});

		if (AttachedComponents.Num() > 1)
		{
			AddError(FString::Printf(TEXT("Received %d components while only 1 expected"), AttachedComponents.Num()));
		}
		else if (AttachedComponents.Num() == 1)
		{
			AttachedComponent = Cast<USpatialTestAttachedComponentReplicationComponent>(AttachedComponents[0]);
		}
		RequireTrue(IsValid(AttachedComponent), TEXT("Received attached component"));

		if (IsValid(AttachedComponent))
		{
			AssertEqual_Int(AttachedComponent->ReplicatedValue, SpatialTestAttachedComponentReplicationValues::InitialValue,
							TEXT("Component's value is initialized correctly"));
		}

		constexpr float ComponentPollDuration = 3.0f;

		// We need to run this step for some time to catch test conditions failing at a later date,
		// for example, two test components appearing on clients.
		RequireCompare_Float(TimeInStep, EComparisonMethod::Greater_Than, ComponentPollDuration,
							 FString::Printf(TEXT("Step ran for %f secs"), ComponentPollDuration));
	});

	AddStep(DescribeTestType(TestType) + TEXT("Modify replicated value on the component"), FWorkerDefinition::Server(1), nullptr, [this] {
		AssertTrue(LevelPlacedActor->HasAuthority(), TEXT("Server 1 has authority over the actor"));
		AttachedComponent->ReplicatedValue = SpatialTestAttachedComponentReplicationValues::ChangedValue;
		FinishStep();
	});

	AddWaitingStep(DescribeTestType(TestType) + TEXT("Check that the updated value is received"), FWorkerDefinition::AllWorkers, [this] {
		RequireEqual_Int(AttachedComponent->ReplicatedValue, SpatialTestAttachedComponentReplicationValues::ChangedValue,
						 TEXT("Updated value received"));
	});
}
