#include "SpatialGDK/SpatialTestReplicatedComponentOnNonReplicatedActor/SpatialTestReplicatedComponentOnNonReplicatedActor.h"

#include "Algo/Copy.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialFunctionalTestStep.h"
#include "TestWorkerSettings.h"

const FVector LevelPlacedTestActorPosition{ -100, 0, 0 };
const FVector DynamicallySpawnedTestActorPosition{ 100, 0, 0 };

FVector GetDynamicTestActorPosition(const FWorkerDefinition& Definition)
{
	switch (Definition.Id)
	{
	case 1:
		return { -100, -100, 0 };
	case 2:
		return { 100, 100, 0 };
	default:
		checkNoEntry();
	}
	return {};
}

UReplicatedComponentOnNonReplicatedActorGeneratedMap::UReplicatedComponentOnNonReplicatedActorGeneratedMap()
	: Super(EMapCategory::CI_PREMERGE, TEXT("ReplicatedComponentOnNonReplicatedActor"))
{
}

void UReplicatedComponentOnNonReplicatedActorGeneratedMap::CreateCustomContentForMap()
{
	Super::CreateCustomContentForMap();

	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the tests
	AddActorToLevel<ATestNonReplicatedActor>(
		CurrentLevel, FTransform(LevelPlacedTestActorPosition)); // Seems like this position is required so that the LB plays nicely?

	AddActorToLevel<ASpatialTestReplicatedComponentOnNonReplicatedActor>(CurrentLevel, FTransform::Identity);

	ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(CurrentLevel->GetWorldSettings());

	if (ensure(IsValid(WorldSettings)))
	{
		WorldSettings->SetMultiWorkerSettingsClass(UTest1x2FullInterestWorkerSettings::StaticClass());
	}
}

UTestReplicatedComponent::UTestReplicatedComponent()
{
	SetIsReplicatedByDefault(true);
}

void UTestReplicatedComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Dummy);
}

ATestNonReplicatedActor::ATestNonReplicatedActor()
{
	bReplicates = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootCoponent"));
	ReplicatedDefaultReference = CreateDefaultSubobject<UTestReplicatedComponent>(TEXT("ReplicatedReferencedDefaultComponent"));
	NonReplicatedDefaultReference = CreateDefaultSubobject<UTestReplicatedComponent>(TEXT("NonReplicatedReferencedDefaultComponent"));
}

void ATestNonReplicatedActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ReplicatedDefaultReference);
	DOREPLIFETIME(ThisClass, ReplicatedRuntimeReference);
}

void ATestNonReplicatedActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ReplicatedRuntimeReference = NewObject<UTestReplicatedComponent>(this);
	ReplicatedRuntimeReference->RegisterComponent();

	NonReplicatedRuntimeReference = NewObject<UTestReplicatedComponent>(this);
	NonReplicatedRuntimeReference->RegisterComponent();
}

ATestReplicatedActor::ATestReplicatedActor()
{
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void ATestReplicatedActor::OnAuthorityGained()
{
	Super::OnAuthorityGained();
}

void ATestReplicatedActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ActorInfos);
	DOREPLIFETIME(ThisClass, OriginWorkerDefinition);
}

void ASpatialTestReplicatedComponentOnNonReplicatedActor::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("Spawn dynamic nonreplicated actor"), FWorkerDefinition::AllServers, nullptr, [this] {
		GetWorld()->SpawnActor<ATestNonReplicatedActor>(DynamicallySpawnedTestActorPosition, FRotator::ZeroRotator);
		FinishStep();
	});

	AddStep(TEXT("Delay"), FWorkerDefinition::AllWorkers, nullptr, nullptr, [this](float dt) {
		if (counter > 5)
		{
			FinishStep();
			counter = 0;
		}
		counter += dt;
	});

	AddStep(TEXT("Spawn dynamic replicated actor"), FWorkerDefinition::AllServers, nullptr, [this] {
		GetWorld()->SpawnActor<ATestReplicatedActor>(GetDynamicTestActorPosition(GetLocalFlowController()->GetWorkerDefinition()),
													 FRotator::ZeroRotator);
		FinishStep();
	});

	AddStep(TEXT("Delay"), FWorkerDefinition::AllWorkers, nullptr, nullptr, [this](float dt) {
		if (counter > 5)
		{
			FinishStep();
			counter = 0;
		}
		counter += dt;
	});

	AddStep(TEXT("Find nonreplicated actors"), FWorkerDefinition::AllWorkers, nullptr, nullptr, [this](float) {
		TestActors.Empty();
		Algo::Copy(TActorRange<ATestNonReplicatedActor>(GetWorld()), TestActors);
		TestActors.Sort([](const ATestNonReplicatedActor& Lhs, const ATestNonReplicatedActor& Rhs) {
			return Lhs.GetActorLocation().X < Rhs.GetActorLocation().X;
		});
		const int32 ExpectedTestActorCount = GetLocalWorkerType() == ESpatialFunctionalTestWorkerType::Server ? 2 : 1;
		RequireEqual_Int(TestActors.Num(), ExpectedTestActorCount, TEXT("Correct amount of test actors"));
		FinishStep();
	});

	AddStep(TEXT("Delay"), FWorkerDefinition::AllWorkers, nullptr, nullptr, [this](float dt) {
		if (counter > 5)
		{
			FinishStep();
			counter = 0;
		}
		counter += dt;
	});

	AddStep(TEXT("Find replicated actors"), FWorkerDefinition::AllWorkers, nullptr, nullptr, [this](float) {
		ReplicatedActors.Empty();
		Algo::Copy(TActorRange<ATestReplicatedActor>(GetWorld()), ReplicatedActors);
		ReplicatedActors.Sort([](const ATestReplicatedActor& Lhs, const ATestReplicatedActor& Rhs) {
			return Lhs.GetActorLocation().X < Rhs.GetActorLocation().X;
		});
		const int32 ExpectedTestActorCount = 2;
		RequireEqual_Int(ReplicatedActors.Num(), ExpectedTestActorCount, TEXT("Correct amount of replicated actors"));
		FinishStep();
	});

	AddStep(TEXT("Delay"), FWorkerDefinition::AllWorkers, nullptr, nullptr, [this](float dt) {
		if (counter > 5)
		{
			FinishStep();
			counter = 0;
		}
		counter += dt;
	});
	AddStep(TEXT("Pick locally auth replicated actor"), FWorkerDefinition::AllServers, nullptr, [this] {
		ATestReplicatedActor** LocallyAuthActorPtr = ReplicatedActors.FindByPredicate([](const ATestReplicatedActor* Actor) {
			return Actor->HasAuthority();
		});
		if (AssertTrue(LocallyAuthActorPtr != nullptr, TEXT("Found locally auth actor")))
		{
			LocallyAuthReplicatedActor = *LocallyAuthActorPtr;
		}
		FinishStep();
	});

	AddStep(TEXT("Delay"), FWorkerDefinition::AllWorkers, nullptr, nullptr, [this](float dt) {
		if (counter > 5)
		{
			FinishStep();
			counter = 0;
		}
		counter += dt;
	});

	AddStep(TEXT("Gather all replicated components on non-replicated actors and replicate them"), FWorkerDefinition::AllServers, nullptr,
			[this] {
				LocallyAuthReplicatedActor->ActorInfos = Algo::Accumulate(
					TestActors, LocallyAuthReplicatedActor->ActorInfos,
					[](decltype(LocallyAuthReplicatedActor->ActorInfos) ActorInfos, const ATestNonReplicatedActor* TestActor) {
						ActorInfos.Append({
							{ TestActor->ReplicatedDefaultReference },
							{ TestActor->ReplicatedRuntimeReference },
							{ TestActor->NonReplicatedDefaultReference },
							{ TestActor->NonReplicatedRuntimeReference },
						});
						return ActorInfos;
					});
				FinishStep();
			});

	AddStep(TEXT("Delay"), FWorkerDefinition::AllWorkers, nullptr, nullptr, [this](float dt) {
		if (counter > 5)
		{
			FinishStep();
			counter = 0;
		}
		counter += dt;
	});

	AddStep(TEXT("Check replicated values on clients"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](float) {
		for (const ATestReplicatedActor* ReplicatedActor : ReplicatedActors)
		{
			RequireEqual_Int(ReplicatedActor->ActorInfos.Num(), 8, TEXT("Correct amount of replicated component infos"));
		}

		FinishStep();
	});

	AddStep(TEXT("Delay"), FWorkerDefinition::AllWorkers, nullptr, nullptr, [this](float dt) {
		if (counter > 5)
		{
			FinishStep();
			counter = 0;
		}
		counter += dt;
	});
}
