// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DormancyTest.h"
#include "DormancyTestActor.h"
#include "EngineUtils.h"

AActor* ADormancyTest::CreateDormancyTestActor()
{
	return GetWorld()->SpawnActor<ADormancyTestActor>({ 0.0f, 0.0f, 0.0f }, FRotator::ZeroRotator);
}

void ADormancyTest::CheckDormancyAndRepProperty(const TEnumAsByte<enum ENetDormancy> ExpectedNetDormancy, const int ExpectedTestIntProp)
{
	for (TActorIterator<ADormancyTestActor> Iter(GetWorld()); Iter; ++Iter)
	{
		ADormancyTestActor* DormancyTestActor = *Iter;
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
	RequireEqual_Int(Counter, ExpectedCount, TEXT("Number of TestDormancyActors in client world"));
}

FString ADormancyTest::NetDormancyToString(TEnumAsByte<enum ENetDormancy> InNetDormancy)
{
	switch (InNetDormancy)
	{
	case ENetDormancy::DORM_Never:
		return TEXT("DORM_Never");
	case ENetDormancy::DORM_Awake:
		return TEXT("DORM_Awake");
	case ENetDormancy::DORM_DormantAll:
		return TEXT("DORM_DormantAll");
	case ENetDormancy::DORM_DormantPartial:
		return TEXT("DORM_DormantPartial");
	case ENetDormancy::DORM_Initial:
		return TEXT("DORM_Initial");
	default:
		return TEXT("UNKOWN");
		break;
	}
}
