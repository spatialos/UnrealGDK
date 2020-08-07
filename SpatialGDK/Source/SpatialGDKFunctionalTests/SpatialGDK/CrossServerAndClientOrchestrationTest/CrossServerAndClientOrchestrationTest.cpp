// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerAndClientOrchestrationTest.h"

#include "CrossServerAndClientOrchestrationFlowController.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Net/UnrealNetwork.h"

/**
 * This test tests server and client steps run in the right worker and can modify test data via CrossServer RPCs.
 *
 * The test includes 2 servers and 2 client workers. All workers will try to set some state in the Test actor via CrossServer RPC and ensure
 * they have received updates done by other workers. The flow is as follows:
 *  - Setup:
 *    - All server and clients set a flag in the test actor via CrossServer RPC
 *  - Test:
 *    - Each server and client, individually, verifies they are running in the right FlowController and worker context
 *    - Each server and client, individually, verifies they can read all values set by the other workers
 */
ACrossServerAndClientOrchestrationTest::ACrossServerAndClientOrchestrationTest()
{
	Author = "Jose";
	Description = TEXT("Test the test flow in a zoned environment");

	FlowControllerActorClass = ACrossServerAndClientOrchestrationFlowController::StaticClass();
}

void ACrossServerAndClientOrchestrationTest::BeginPlay()
{
	Super::BeginPlay();

	ClientWorkerSetValues.SetNum(GetNumRequiredClients());
	ServerWorkerSetValues.SetNum(GetNumExpectedServers());

	{
		// Step 1 - Set all server values
		AddStep(TEXT("Servers_SetupSetValue"), FWorkerDefinition::AllServers, nullptr, [](ASpatialFunctionalTest* NetTest) {
			// Send CrossServer RPC to Test actor to set the flag for this server flow controller instance
			ACrossServerAndClientOrchestrationTest* CrossServerTest = Cast<ACrossServerAndClientOrchestrationTest>(NetTest);
			ASpatialFunctionalTestFlowController* FlowController = CrossServerTest->GetLocalFlowController();
			CrossServerTest->CrossServerSetTestValue(FlowController->WorkerDefinition.Type, FlowController->WorkerDefinition.Id);
			CrossServerTest->FinishStep();
		});
	}
	{
		// Step 2 - Set all client values
		AddStep(TEXT("Clients_SetupSetValue"), FWorkerDefinition::AllClients, nullptr, [](ASpatialFunctionalTest* NetTest) {
			// Send Server RPC via flow controller to set the Test actor flag for this client flow controller instance
			ACrossServerAndClientOrchestrationFlowController* FlowController =
				Cast<ACrossServerAndClientOrchestrationFlowController>(NetTest->GetLocalFlowController());
			FlowController->ServerClientReadValueAck();
			NetTest->FinishStep();
		});
	}
	{
		// Step 3 - Verify steps for server 1 run in right context and can read all values set in test by other workers
		AddStep(
			TEXT("Server1_Validate"), FWorkerDefinition::Server(1), nullptr,
			[](ASpatialFunctionalTest* NetTest) {
				ACrossServerAndClientOrchestrationTest* CrossServerTest = Cast<ACrossServerAndClientOrchestrationTest>(NetTest);
				CrossServerTest->Assert_ServerStepIsRunningInExpectedEnvironment(1, CrossServerTest->GetLocalFlowController());
			},
			[](ASpatialFunctionalTest* NetTest, float DeltaTime) {
				ACrossServerAndClientOrchestrationTest* CrossServerTest = Cast<ACrossServerAndClientOrchestrationTest>(NetTest);
				if (CrossServerTest->CheckAllValuesHaveBeenSetAndCanBeLocallyRead())
				{
					CrossServerTest->FinishStep();
				}
			},
			5.0f);
	}
	{
		// Step 4 - Verify steps for server 2 run in right context and can read all values set in test by other workers
		AddStep(
			TEXT("Server2_Validate"), FWorkerDefinition::Server(2), nullptr,
			[](ASpatialFunctionalTest* NetTest) {
				ACrossServerAndClientOrchestrationTest* CrossServerTest = Cast<ACrossServerAndClientOrchestrationTest>(NetTest);
				CrossServerTest->Assert_ServerStepIsRunningInExpectedEnvironment(2, CrossServerTest->GetLocalFlowController());
			},
			[](ASpatialFunctionalTest* NetTest, float DeltaTime) {
				ACrossServerAndClientOrchestrationTest* CrossServerTest = Cast<ACrossServerAndClientOrchestrationTest>(NetTest);
				if (CrossServerTest->CheckAllValuesHaveBeenSetAndCanBeLocallyRead())
				{
					CrossServerTest->FinishStep();
				}
			},
			5.0f);
	}
	{
		// Step 5 - Verify steps for client 1 run in right context and can read all values set in test by other workers
		AddStep(
			TEXT("Client1_Validate"), FWorkerDefinition::Client(1), nullptr,
			[](ASpatialFunctionalTest* NetTest) {
				ACrossServerAndClientOrchestrationTest* CrossServerTest = Cast<ACrossServerAndClientOrchestrationTest>(NetTest);
				CrossServerTest->Assert_ClientStepIsRunningInExpectedEnvironment(1, CrossServerTest->GetLocalFlowController());
			},
			[](ASpatialFunctionalTest* NetTest, float DeltaTime) {
				ACrossServerAndClientOrchestrationTest* CrossServerTest = Cast<ACrossServerAndClientOrchestrationTest>(NetTest);
				if (CrossServerTest->CheckAllValuesHaveBeenSetAndCanBeLocallyRead())
				{
					CrossServerTest->FinishStep();
				}
			},
			5.0f);
	}
	{
		// Step 6 - Verify steps for client 2 run in right context and can read all values set in test by other workers
		AddStep(
			TEXT("Client2_Validate"), FWorkerDefinition::Client(2), nullptr,
			[](ASpatialFunctionalTest* NetTest) {
				ACrossServerAndClientOrchestrationTest* CrossServerTest = Cast<ACrossServerAndClientOrchestrationTest>(NetTest);
				CrossServerTest->Assert_ClientStepIsRunningInExpectedEnvironment(2, CrossServerTest->GetLocalFlowController());
			},
			[](ASpatialFunctionalTest* NetTest, float DeltaTime) {
				ACrossServerAndClientOrchestrationTest* CrossServerTest = Cast<ACrossServerAndClientOrchestrationTest>(NetTest);
				if (CrossServerTest->CheckAllValuesHaveBeenSetAndCanBeLocallyRead())
				{
					CrossServerTest->FinishStep();
				}
			},
			5.0f);
	}
}

void ACrossServerAndClientOrchestrationTest::CrossServerSetTestValue_Implementation(ESpatialFunctionalTestWorkerType ControllerType,
																					uint8 ChangedInstance)
{
	uint8 FlagIndex = ChangedInstance - 1;
	if (ControllerType == ESpatialFunctionalTestWorkerType::Client)
	{
		if (FlagIndex >= ClientWorkerSetValues.Num())
		{
			// ignore
			return;
		}
		ClientWorkerSetValues[FlagIndex] = true;
	}
	else
	{
		if (FlagIndex >= ServerWorkerSetValues.Num())
		{
			// ignore
			return;
		}
		ServerWorkerSetValues[FlagIndex] = true;
	}
}

void ACrossServerAndClientOrchestrationTest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACrossServerAndClientOrchestrationTest, ClientWorkerSetValues);
	DOREPLIFETIME(ACrossServerAndClientOrchestrationTest, ServerWorkerSetValues);
}

void ACrossServerAndClientOrchestrationTest::Assert_ServerStepIsRunningInExpectedEnvironment(
	int InstanceToRunIn, ASpatialFunctionalTestFlowController* FlowController)
{
	// Check if we are using loadbalancing configuration with multiple server workers
	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetNetDriver());

	bool bUsingLoadbalancing = SpatialNetDriver != nullptr && SpatialNetDriver->LoadBalanceStrategy != nullptr;

	VirtualWorkerId LocalWorkerId = bUsingLoadbalancing ? SpatialNetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId() : 1;
	InstanceToRunIn = GetNumExpectedServers() == 1 ? 1 : InstanceToRunIn;

	// Check Step is running in expected controller instance
	AssertEqual_Int(FlowController->WorkerDefinition.Id, InstanceToRunIn, TEXT("Step executing in expected FlowController instance"), this);

	// Check Step is running in expected worker instance
	AssertEqual_Int(LocalWorkerId, InstanceToRunIn, TEXT("Step executing in expected Worker instance"), this);
}

void ACrossServerAndClientOrchestrationTest::Assert_ClientStepIsRunningInExpectedEnvironment(
	int InstanceToRunIn, ASpatialFunctionalTestFlowController* FlowController)
{
	// Check Step is running in expected controller instance
	// We can't check against clients as clients don't have natural logical IDs, Controllers are mapped by login order
	AssertEqual_Int(FlowController->WorkerDefinition.Id, InstanceToRunIn, TEXT("Step executing in expected FlowController instance"), this);
}

bool ACrossServerAndClientOrchestrationTest::CheckAllValuesHaveBeenSetAndCanBeLocallyRead()
{
	for (int i = 0; i < ServerWorkerSetValues.Num(); i++)
	{
		if (!ServerWorkerSetValues[i])
		{
			return false;
		}
	}
	for (int i = 0; i < ClientWorkerSetValues.Num(); i++)
	{
		if (!ClientWorkerSetValues[i])
		{
			return false;
		}
	}

	return true;
}
