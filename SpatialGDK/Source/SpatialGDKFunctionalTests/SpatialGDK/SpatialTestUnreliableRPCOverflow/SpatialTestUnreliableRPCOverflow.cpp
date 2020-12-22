// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestUnreliableRPCOverflow.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKSettings.h"
#include "UnreliableRPCTestActor.h"

/**
 * This tests that when unreliable RPCs overflow, it will drop the older RPCs to sent the newer ones.
 * Tests UNR-4360
 * This test needs 1 Server and 1 Client workers.
 *
 * The flow is as follows:
 * - Setup:
 *  - Spawn an AUnreliableRPCTestActor in the server and give ownership to the Client
 * - Test:
 *  - Sent GetRPCRingBufferSize() + 2 unreliable RPCs from the server to the client.
 *  - Check that the 2 oldest RPCs have been dropped, and the 2 most recent have been received.
 */

ASpatialTestUnreliableRPCOverflow::ASpatialTestUnreliableRPCOverflow()
	: Super()
	, RPCLimitCount(GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(ERPCType::ClientUnreliable))
{
	Author = "Antoine Cordelle (antoine.cordelle@improbable.io)";
	Description = TEXT("This tests that when unreliable RPCs overflow, it will drop the older RPCs to sent the newer ones.");
}

void ASpatialTestUnreliableRPCOverflow::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("The Server spawns one AUnreliableRPCTestActor and gives ownership to the Client"), FWorkerDefinition::Server(1), nullptr,
			[this]() {
				TestActor = GetWorld()->SpawnActor<AUnreliableRPCTestActor>(FVector(0.0f, 0.0f, 50.0f), FRotator::ZeroRotator);
				RegisterAutoDestroyActor(TestActor);

				ASpatialFunctionalTestFlowController* Client1FlowController =
					GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);

				AssertIsValid(TestActor, "Actor exists", this);
				if (TestActor)
				{
					TestActor->SetReplicates(true);
					TestActor->SetOwner(Client1FlowController->GetOwner());
				}

				FinishStep();
			});

	AddStep(
		TEXT("Make sure client has ownership of Actor"), FWorkerDefinition::Client(1), nullptr, nullptr,
		[this](float DeltaTime) {
			if (IsValid(TestActor) && TestActor->GetOwner() == GetLocalFlowController()->GetOwner())
			{
				FinishStep();
			}
		},
		5.0f);

	AddStep(
		TEXT("Sent GetRPCRingBufferSize() + 2 unreliable RPCs from the server to the client"), FWorkerDefinition::Server(1),
		[this]() -> bool {
			return IsValid(TestActor);
		},
		[this]() {
			TestActor->LaunchRPCs(RPCLimitCount + 2);
			FinishStep();
		});

	AddStep(
		TEXT("Check that the 2 oldest RPCs have been dropped, and the 2 most recent have been received"), FWorkerDefinition::Client(1),
		[this]() -> bool {
			return IsValid(TestActor);
		},
		[this]() {
			const auto& Array = TestActor->GetArray();
			AssertTrue(
				Array.Num() == RPCLimitCount,
				FString::Printf(TEXT("Only RPCLimitCount = %i RPCs should be received. The first two will be dropped"), RPCLimitCount));
			AssertFalse(Array.Contains(0) || Array.Contains(1),
						TEXT("The first 2 RPCs should be dropped because more recent RPCs overflowed"));
			AssertTrue(Array.Contains(RPCLimitCount) && Array.Contains(RPCLimitCount + 1),
					   TEXT("The last 2 RPCs should be sent even if overflowing as they are the most recent"));
			FinishStep();
		});
}

void ASpatialTestUnreliableRPCOverflow::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTestUnreliableRPCOverflow, TestActor);
}
