// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EntityInteractionTest.h"
#include "EntityInteractionTestActor.h"
#include "Kismet/GameplayStatics.h"

#include "SpatialGDKSettings.h"

/**
 * This test goes through the various methods we have added to handle remote action and interaction between Actors.
 * It tests two things :
 * - That all calls eventually make their way to the target actor
 * - That calls that are expected to short circuit do so, and the ones that are not result in a remote call.
 * The map is setup with two workers, each with auth over two actors. Both workers run symmetrical code.
 * So for each RPC emitted, the symmetrical actor is expected to emit a similar one.
 * This is how we test that the "delayed" RPCs have made their way through the network.
 * This test is only relevant when the routing worker is enabled in the GDK settings.
 */

ASpatialEntityInteractionTest::ASpatialEntityInteractionTest()
	: Super()
{
	Author = "Nicolas";
	Description = TEXT("");
}

#define EXPECT_IMMEDIATE(Actor, FunctionName, Context)                                                                                     \
	do                                                                                                                                     \
	{                                                                                                                                      \
		FString DebugString = FString::Printf(TEXT("Immediate execution of a %s on %s : %s"), *FunctionName, *Actor->GetName(), Context);  \
		AssertTrue(Actor->Steps.Contains(NumSteps) && Actor->Steps[NumSteps] == FunctionName, DebugString);                                \
		++NumSteps;                                                                                                                        \
	} while (false)

#define EXPECT_DELAYED(Actor, FunctionName, Context)                                                                                       \
	do                                                                                                                                     \
	{                                                                                                                                      \
		FString DebugString = FString::Printf(TEXT("Delayed execution of a %s on %s : %s"), *FunctionName, *Actor->GetName(), Context);    \
		AssertFalse(Actor->Steps.Contains(NumSteps), DebugString);                                                                         \
		ExpectedResult.Add(NumSteps++, FunctionName);                                                                                      \
	} while (false)

void ASpatialEntityInteractionTest::PrepareTest()
{
	Super::PrepareTest();

	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();
	if (Settings->CrossServerRPCImplementation != ECrossServerRPCImplementation::RoutingWorker)
	{
		AddStep(
			"DummyStep", FWorkerDefinition::AllWorkers, nullptr,
			[this] {
				FinishStep();
			},
			nullptr);
		return;
	}

	AddStep(
		"Discovery", FWorkerDefinition::AllServers, nullptr,
		[this]() {
			NumSteps = 0;
			ExpectedResult.Empty();
			for (auto& Actor : LocalActors)
			{
				Actor = nullptr;
			}
			for (auto& Actor : RemoteActors)
			{
				Actor = nullptr;
			}
			TArray<AActor*> TestActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEntityInteractionTestActor::StaticClass(), TestActors);
			for (AActor* Actor : TestActors)
			{
				AEntityInteractionTestActor* TestActor = CastChecked<AEntityInteractionTestActor>(Actor);

				if (ensure(TestActor->Index < 2))
				{
					if (TestActor->HasAuthority())
					{
						LocalActors[TestActor->Index] = TestActor;
					}
					else
					{
						RemoteActors[TestActor->Index] = TestActor;
					}
				}
			}
			for (auto Actor : LocalActors)
			{
				AssertTrue(Actor != nullptr, TEXT("Missing test actors"));
			}
			for (auto Actor : RemoteActors)
			{
				AssertTrue(Actor != nullptr, TEXT("Missing test actors"));
			}
			FinishStep();
		},
		nullptr);

	AddStep(
		"Send RPCs", FWorkerDefinition::AllServers, nullptr,
		[this]() {
			AActor::ExecuteWithNetWriteFence(*RemoteActors[0], *LocalActors[0], &AEntityInteractionTestActor::TestNetWriteFence, NumSteps);
			EXPECT_IMMEDIATE(LocalActors[0], AEntityInteractionTestActor::s_NetWriteFenceName, TEXT("Remote dependent"));

			AActor::ExecuteWithNetWriteFence(*LocalActors[1], *LocalActors[0], &AEntityInteractionTestActor::TestNetWriteFence, NumSteps);
			EXPECT_DELAYED(LocalActors[0], AEntityInteractionTestActor::s_NetWriteFenceName, TEXT("Local dependent"));

			LocalActors[1]->SendCrossServerRPC(*LocalActors[0], &AEntityInteractionTestActor::TestReliable, NumSteps);
			EXPECT_IMMEDIATE(LocalActors[0], AEntityInteractionTestActor::s_ReliableName, TEXT("Local Receiver"));

			LocalActors[1]->SendCrossServerRPC(*RemoteActors[0], &AEntityInteractionTestActor::TestReliable, NumSteps);
			EXPECT_DELAYED(RemoteActors[0], AEntityInteractionTestActor::s_ReliableName, TEXT("Remote Receiver"));

			LocalActors[0]->TestUnreliable(NumSteps);
			EXPECT_IMMEDIATE(LocalActors[0], AEntityInteractionTestActor::s_UnreliableName, TEXT("Local Receiver"));
			RemoteActors[0]->TestUnreliable(NumSteps);
			EXPECT_DELAYED(RemoteActors[0], AEntityInteractionTestActor::s_UnreliableName, TEXT("Remote Receiver"));

			LocalActors[0]->TestUnordered(NumSteps);
			EXPECT_IMMEDIATE(LocalActors[0], AEntityInteractionTestActor::s_UnorderedName, TEXT("Local Receiver"));
			RemoteActors[0]->TestUnordered(NumSteps);
			EXPECT_DELAYED(RemoteActors[0], AEntityInteractionTestActor::s_UnorderedName, TEXT("Remote Receiver"));

			LocalActors[1]->SendCrossServerRPC(*LocalActors[0], &AEntityInteractionTestActor::TestNoLoopback, NumSteps);
			EXPECT_DELAYED(LocalActors[0], AEntityInteractionTestActor::s_NoLoopbackName, TEXT("Local Receiver"));

			LocalActors[1]->SendCrossServerRPC(*RemoteActors[0], &AEntityInteractionTestActor::TestNoLoopback, NumSteps);
			EXPECT_DELAYED(RemoteActors[0], AEntityInteractionTestActor::s_NoLoopbackName, TEXT("Remote Receiver"));

			FinishStep();
		},
		nullptr, 5.0);

	AddStep(
		"Check delayed RPCs", FWorkerDefinition::AllServers,
		[this]() {
			if (LocalActors[0]->Steps.Num() == NumSteps)
			{
				return true;
			}
			return false;
		},
		[this]() {
			for (auto const& Expected : ExpectedResult)
			{
				FString DebugString("Successful delayed execution of a ");
				DebugString += Expected.Value;
				AssertTrue(LocalActors[0]->Steps.Contains(Expected.Key) && LocalActors[0]->Steps[Expected.Key] == Expected.Value,
						   DebugString);
			}

			FinishStep();
		},
		nullptr, 5.0);
}
