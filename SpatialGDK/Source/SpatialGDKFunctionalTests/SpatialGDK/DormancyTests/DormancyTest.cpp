// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DormancyTest.h"

#include "DormancyTestActor.h"

AActor* ADormancyTest::CreateDormancyTestActor()
{
	return SpawnActor<ADormancyTestActor>({ 0.0f, 0.0f, 0.0f });
}

void ADormancyTest::RequireDormancyTestState(const TEnumAsByte<enum ENetDormancy> ExpectedNetDormancy, const int ExpectedTestIntProp,
											 const int ExpectedCount)
{
	int Counter = 0;
	for (TActorIterator<ADormancyTestActor> Iter(GetWorld()); Iter; ++Iter)
	{
		const ADormancyTestActor* DormancyTestActor = *Iter;
		RequireEqual_Int(DormancyTestActor->NetDormancy, ExpectedNetDormancy, TEXT("Dormancy on ADormancyTestActor"));
		RequireEqual_Int(DormancyTestActor->TestIntProp, ExpectedTestIntProp, TEXT("TestIntProp on ADormancyTestActor"));
		Counter++;
	}
	RequireEqual_Int(Counter, ExpectedCount, TEXT("Number of DormancyTestActors in world"));
}

void ADormancyTest::DestroyDormancyTestActors()
{
	for (TActorIterator<ADormancyTestActor> Iter(GetWorld()); Iter; ++Iter)
	{
		if (Iter->HasAuthority())
		{
			Iter->Destroy();
		}
	}
}
