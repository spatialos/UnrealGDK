
// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "UnresolvedReferenceTest.h"
#include "UnresolvedReferenceTestActor.h"
#include <Runtime/Core/Public/Algo/Count.h>

AUnresolvedReferenceTest::AUnresolvedReferenceTest()
	: Super()
{
	Author = TEXT("Ajanth");
	Description = TEXT(
		"Tests what happens when structs with references to actors whose entity have not been created yet are replicated. \
						The unresolved references should evenetually be resolved");
}

void AUnresolvedReferenceTest::PrepareTest()
{
	Super::PrepareTest();

	// Create actor holding references to the other actors
	AddStep(TEXT("MakeReferenceActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		AUnresolvedReferenceTestActor* RefsActor =
			GetWorld()->SpawnActor<AUnresolvedReferenceTestActor>(FVector::ZeroVector, FRotator::ZeroRotator);
		RegisterAutoDestroyActor(RefsActor);

		FinishStep();
	});

	// Wait for actor which will hold references to replicate to the clients
	AddStep(
		TEXT("ClientWaitForReplication"), FWorkerDefinition::AllClients, nullptr,
		[this]() {
			FTimerHandle DelayTimerHandle;
			FTimerManager& TimerManager = GetWorld()->GetTimerManager();
			TimerManager.SetTimer(
				DelayTimerHandle,
				[this]() {
					FinishStep();
				},
				0.5f, false);
		},
		nullptr, 5.0f);

	// Add actor references to actor holding references
	AddStep(TEXT("MakeActorReferences"), FWorkerDefinition::Server(1), nullptr, [this]() {
		for (AUnresolvedReferenceTestActor* RefsActor : TActorRange<AUnresolvedReferenceTestActor>(GetWorld()))
		{
			// 3 actors are enough to check if the unresolved references are resolved
			for (int i = 0; i < 3; i++)
			{
				AReplicatedTestActorBase* Actor =
					GetWorld()->SpawnActor<AReplicatedTestActorBase>(FVector::ZeroVector, FRotator::ZeroRotator, FActorSpawnParameters());
				RefsActor->ActorRefs.Add(Actor);
				RegisterAutoDestroyActor(Actor);
			}
		}
		FinishStep();
	});

	// Check that none of the actor references are null - step will timeout after 5 seconds
	AddStep(
		TEXT("CheckNoNullReferences"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float deltaTime) {
			for (const AUnresolvedReferenceTestActor* RefsActor : TActorRange<AUnresolvedReferenceTestActor>(GetWorld()))
			{
				RequireCompare_Int(RefsActor->ActorRefs.Num(), EComparisonMethod::Greater_Than, 0,
								   TEXT("ActorRefs array length should be non-zero"));

				RequireEqual_Int(Algo::Count(RefsActor->ActorRefs, nullptr), 0, TEXT("There are no unresolved actors in ActorRefs"));
			}

			FinishStep();
		},
		5.0f);
}
