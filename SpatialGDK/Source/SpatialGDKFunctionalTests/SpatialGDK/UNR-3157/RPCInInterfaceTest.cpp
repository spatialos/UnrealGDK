// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "RPCInInterfaceTest.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTestFlowController.h"

/**
 * This test ensures that an RPC function declared in an interface can be called in Spatial to ensure parity with native Unreal.
 * It creates an actor, transfers ownership and then calls a client RPC on that actor. Finally, it verifies that the RPC was received.
 */
ARPCInInterfaceTest::ARPCInInterfaceTest()
{
	Author = "Andreas";
	Description = TEXT("Test RPCs in interfaces");
}

void ARPCInInterfaceTest::BeginPlay()
{
	Super::BeginPlay();

	{ // Step 1 - Create actor
		AddStep(TEXT("ServerCreateActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
			ASpatialFunctionalTestFlowController* Client1FlowController =
				GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);

			TestActor = GetWorld()->SpawnActor<ARPCInInterfaceActor>();
			AssertIsValid(TestActor, "Actor exists", this);
			if (TestActor)
			{
				TestActor->SetReplicates(true);
				TestActor->SetOwner(Client1FlowController->GetOwner());
			}

			FinishStep();
		});
	}
	{ // Step 2 - Make sure client has ownership of Actor
		AddStep(TEXT("ClientCheckOwnership"), FWorkerDefinition::Client(1), nullptr, nullptr,
				[this](float DeltaTime) {
					if (IsValid(TestActor) && TestActor->GetOwner() == GetLocalFlowController()->GetOwner())
					{
						FinishStep();
					}
				});
	}
	{ // Step 3 - Call client RPC on interface
		AddStep(
			TEXT("ServerCallRPC"), FWorkerDefinition::Server(1),
			[this]() -> bool {
				return IsValid(TestActor);
			},
			[this]() {
				TestActor->RPCInInterface();
				FinishStep();
			});
	}
	{ // Step 4 - Check RPC was received on client
		AddStep(
			TEXT("ClientCheckRPC"), FWorkerDefinition::AllClients,
			[this]() -> bool {
				return IsValid(TestActor);
			},
			nullptr,
			[this](float DeltaTime) {
				if (TestActor->GetOwner() == GetLocalFlowController()->GetOwner())
				{
					if (TestActor->bRPCReceived)
					{
						AssertTrue(TestActor->bRPCReceived, "RPC was received", this);
						FinishStep();
					}
				}
				else
				{
					StepTimer += DeltaTime;
					if (StepTimer > 1.0f) // we give it up to 1s to make sure it wasn't received
					{
						StepTimer = 0.0f;
						AssertFalse(TestActor->bRPCReceived, "RPC not received on non-owning client", this);
						FinishStep();
					}
				}
			});
	}
}

void ARPCInInterfaceTest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARPCInInterfaceTest, TestActor);
}
