
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
						The unresolved references should eventually be resolved");
}

void AUnresolvedReferenceTest::PrepareTest()
{
	Super::PrepareTest();

	// Create actor holding references to the other actors
	AddStep(TEXT("MakeReferenceActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		SpawnActor<AUnresolvedReferenceTestActor>();

		FinishStep();
	});

	// Wait for actor which will hold references to replicate to the clients
	AddStep(TEXT("ClientWaitForReplication"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](float DeltaTime) {
		RequireEqual_Int(CountActors<AUnresolvedReferenceTestActor>(GetWorld()), 1, TEXT("There should be 1 RefActor"));
		FinishStep();
	});

	// Add actor references to actor holding references
	AddStep(TEXT("MakeActorReferences"), FWorkerDefinition::Server(1), nullptr, [this]() {
		for (AUnresolvedReferenceTestActor* RefsActor : TActorRange<AUnresolvedReferenceTestActor>(GetWorld()))
		{
			for (int i = 0; i < NumActors; i++)
			{
				AReplicatedTestActorBase* Actor = SpawnActor<AReplicatedTestActorBase>();
				RefsActor->ActorRefs.Add(Actor);
			}
		}
		FinishStep();
	});

	// Check that none of the actor references are null - step will timeout after 5 seconds
	AddStep(
		TEXT("CheckNoNullReferences"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			for (const AUnresolvedReferenceTestActor* RefsActor : TActorRange<AUnresolvedReferenceTestActor>(GetWorld()))
			{
				RequireCompare_Int(RefsActor->ActorRefs.Num(), EComparisonMethod::Equal_To, NumActors,
								   FString::Printf(TEXT("Number of actors should be %d"), NumActors));

				RequireEqual_Int(Algo::Count(RefsActor->ActorRefs, nullptr), 0, TEXT("There are no unresolved actors in ActorRefs"));
			}

			FinishStep();
		},
		5.0f);
}
