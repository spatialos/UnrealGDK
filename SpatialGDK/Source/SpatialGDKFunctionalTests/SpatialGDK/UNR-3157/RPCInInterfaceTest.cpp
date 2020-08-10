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
		AddStep(TEXT("ServerCreateActor"), FWorkerDefinition::Server(1), nullptr, [](ASpatialFunctionalTest* NetTest) {
			ARPCInInterfaceTest* Test = Cast<ARPCInInterfaceTest>(NetTest);

			ASpatialFunctionalTestFlowController* Client1FlowController =
				Test->GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);

			Test->TestActor = Test->GetWorld()->SpawnActor<ARPCInInterfaceActor>();
			Test->AssertIsValid(Test->TestActor, "Actor exists", Test);
			if (Test->TestActor)
			{
				Test->TestActor->SetReplicates(true);
				Test->TestActor->SetOwner(Client1FlowController->GetOwner());
			}

			Test->FinishStep();
		});
	}
	{ // Step 2 - Make sure client has ownership of Actor
		AddStep(TEXT("ClientCheckOwnership"), FWorkerDefinition::Client(1), nullptr, nullptr,
				[](ASpatialFunctionalTest* NetTest, float DeltaTime) {
					ARPCInInterfaceTest* Test = Cast<ARPCInInterfaceTest>(NetTest);
					if (IsValid(Test->TestActor) && Test->TestActor->GetOwner() == Test->GetLocalFlowController()->GetOwner())
					{
						Test->FinishStep();
					}
				});
	}
	{ // Step 3 - Call client RPC on interface
		AddStep(
			TEXT("ServerCallRPC"), FWorkerDefinition::Server(1),
			[](ASpatialFunctionalTest* NetTest) -> bool {
				ARPCInInterfaceTest* Test = Cast<ARPCInInterfaceTest>(NetTest);
				return IsValid(Test->TestActor);
			},
			[](ASpatialFunctionalTest* NetTest) {
				ARPCInInterfaceTest* Test = Cast<ARPCInInterfaceTest>(NetTest);
				Test->TestActor->RPCInInterface();
				Test->FinishStep();
			});
	}
	{ // Step 4 - Check RPC was received on client
		AddStep(
			TEXT("ClientCheckRPC"), FWorkerDefinition::AllClients,
			[](ASpatialFunctionalTest* NetTest) -> bool {
				ARPCInInterfaceTest* Test = Cast<ARPCInInterfaceTest>(NetTest);
				return IsValid(Test->TestActor);
			},
			nullptr,
			[](ASpatialFunctionalTest* NetTest, float DeltaTime) {
				ARPCInInterfaceTest* Test = Cast<ARPCInInterfaceTest>(NetTest);
				if (Test->TestActor->GetOwner() == Test->GetLocalFlowController()->GetOwner())
				{
					if (Test->TestActor->bRPCReceived)
					{
						Test->AssertTrue(Test->TestActor->bRPCReceived, "RPC was received", Test);
						Test->FinishStep();
					}
				}
				else
				{
					Test->StepTimer += DeltaTime;
					if (Test->StepTimer > 1.0f) // we give it up to 1s to make sure it wasn't received
					{
						Test->StepTimer = 0.0f;
						Test->AssertFalse(Test->TestActor->bRPCReceived, "RPC not received on non-owning client", Test);
						Test->FinishStep();
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
