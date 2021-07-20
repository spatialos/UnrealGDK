
// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "UnresolvedReferenceGymTest.h"

AUnresolvedReferenceGymTest::AUnresolvedReferenceGymTest()
	: Super()
{
	Author = "Ajanth";
	Description = TEXT("Tests what happens when structs with references to actors whose entity have not been created yet are replicated.");
}

void AUnresolvedReferenceGymTest::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("MakeActorReferences"), FWorkerDefinition::Server(1), nullptr, [this]() {
		AUnresolvedReferenceGymTestActor* ActorRefsActor =
			GetWorld()->SpawnActor<AUnresolvedReferenceGymTestActor>({ 0.0f, 0.0f, 0.0f }, FRotator::ZeroRotator);
		RegisterAutoDestroyActor(ActorRefsActor);

		for (int i = 0; i < 3; i++)
		{
			AReplicatedTestActorBase* Actor =
				GetWorld()->SpawnActor<AReplicatedTestActorBase>(FVector(0, 0, 0), FRotator::ZeroRotator, FActorSpawnParameters());
			(ActorRefsActor->ActorRefs).Add(Actor);
			RegisterAutoDestroyActor(Actor);
		}

		FinishStep();
	});

	AddStep(TEXT("CheckNoNullReferences"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](float deltaTime) {
		for (TActorIterator<AUnresolvedReferenceGymTestActor> Iter(GetWorld()); Iter; ++Iter)
		{
			int maxAllowedRepNotifies = 15;
			int length = (Iter->ActorRefs).Num();

			RequireEqual_Bool(length > 0, true, TEXT("ActorRefs array length should be non-zero"));

			int numActorsToCheck = length;
			for (int i = 0; i < length; i++)
			{
				if ((Iter->ActorRefs)[i] != nullptr)
				{
					numActorsToCheck--;
				}
			}

			bool bNoNullReferences = (Iter->numRepNotify <= maxAllowedRepNotifies) && (numActorsToCheck == 0);

			RequireEqual_Bool(bNoNullReferences, true, TEXT("There are no null references in ActorRefs"));
		}

		FinishStep();
	});
}
