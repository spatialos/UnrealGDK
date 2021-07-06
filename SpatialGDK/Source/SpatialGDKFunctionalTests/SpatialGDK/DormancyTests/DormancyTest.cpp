// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DormancyTest.h"

#include "DormancyTestActor.h"

AActor* ADormancyTest::CreateDormancyTestActor()
{
	return GetWorld()->SpawnActor<ADormancyTestActor>({ 0.0f, 0.0f, 0.0f }, FRotator::ZeroRotator);
}

void ADormancyTest::CheckDormancyAndRepProperty(const TEnumAsByte<enum ENetDormancy> ExpectedNetDormancy, const int ExpectedTestIntProp)
{
	for (TActorIterator<ADormancyTestActor> Iter(GetWorld()); Iter; ++Iter)
	{
		const ADormancyTestActor* DormancyTestActor = *Iter;
		RequireEqual_Int(DormancyTestActor->NetDormancy, ExpectedNetDormancy, TEXT("Dormancy on ADormancyTestActor"));
		RequireEqual_Int(DormancyTestActor->TestIntProp, ExpectedTestIntProp, TEXT("TestIntProp on ADormancyTestActor"));
	}
}

void ADormancyTest::DestroyDormancyTestActors()
{
	for (TActorIterator<ADormancyTestActor> Iter(GetWorld()); Iter; ++Iter)
	{
		Iter->Destroy();
	}
}

void ADormancyTest::CheckDormancyActorCount(const int ExpectedCount)
{
	int Counter = 0;
	for (TActorIterator<ADormancyTestActor> Iter(GetWorld()); Iter; ++Iter)
	{
		Counter++;
	}
	RequireEqual_Int(Counter, ExpectedCount, TEXT("Number of TestDormancyActors in world"));
}
