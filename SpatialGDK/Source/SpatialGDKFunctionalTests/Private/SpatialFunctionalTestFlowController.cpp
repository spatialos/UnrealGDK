// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialFunctionalTestFlowController.h"

#include "EngineClasses/SpatialGameInstance.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "GameFramework/PlayerController.h"
#include "Interop/SpatialSender.h"
#include "LoadBalancing/LayeredLBStrategy.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTest.h"
#include "SpatialGDKFunctionalTestsPrivate.h"

ASpatialFunctionalTestFlowController::ASpatialFunctionalTestFlowController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
	bAlwaysRelevant = true;

	PrimaryActorTick.TickInterval = 0.0f;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bTickEvenWhenPaused = true;

#if ENGINE_MINOR_VERSION < 24
	bReplicateMovement = false;
#else
	SetReplicatingMovement(false);
#endif
	OwningTest = nullptr;
	bHasAckFinishedTest = true;
	bIsReadyToRunTest = false;
}

void ASpatialFunctionalTestFlowController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialFunctionalTestFlowController, bIsReadyToRunTest);
	DOREPLIFETIME(ASpatialFunctionalTestFlowController, bHasAckFinishedTest);
	DOREPLIFETIME(ASpatialFunctionalTestFlowController, OwningTest);
	DOREPLIFETIME(ASpatialFunctionalTestFlowController, WorkerDefinition);
}

void ASpatialFunctionalTestFlowController::OnAuthorityGained() {}

void ASpatialFunctionalTestFlowController::Tick(float DeltaSeconds)
{
	if (CurrentStep.bIsRunning)
	{
		CurrentStep.Tick(DeltaSeconds);
	}

	// Did it stop now or before the Tick was called?
	if (!CurrentStep.bIsRunning)
	{
		SetActorTickEnabled(false);
		CurrentStep.Reset();
	}
}

void ASpatialFunctionalTestFlowController::CrossServerSetWorkerId_Implementation(int NewWorkerId)
{
	WorkerDefinition.Id = NewWorkerId;
}

void ASpatialFunctionalTestFlowController::OnRep_OwningTest()
{
	// Register replicated flow controllers
	RegisterFlowController();
}

void ASpatialFunctionalTestFlowController::RegisterFlowController()
{
	OwningTest->RegisterFlowController(this);
}

void ASpatialFunctionalTestFlowController::TrySetReadyToRunTest()
{
	if (IsLocalController())
	{
		if (OwningTest->HasPreparedTest())
		{
			SetReadyToRunTest(true);
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick([this]() {
				TrySetReadyToRunTest();
			});
		}
	}
}

void ASpatialFunctionalTestFlowController::ServerSetReadyToRunTest_Implementation(bool bIsReady)
{
	bIsReadyToRunTest = bIsReady;
}

void ASpatialFunctionalTestFlowController::CrossServerStartStep_Implementation(int StepIndex)
{
	// Since we're starting a step, we mark as not Ack that we've finished the test. This is needed
	// for the cases when we run multiple times the same test without a map reload.
	bHasAckFinishedTest = false;
	OwningTest->SetCurrentStepIndex(StepIndex); // Just in case we do not get the replication fast enough
	if (WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
	{
		StartStepInternal(StepIndex);
	}
	else
	{
		ClientStartStep(StepIndex);
	}
}

void ASpatialFunctionalTestFlowController::NotifyStepFinished(const int StepIndex)
{
	if (CurrentStep.bIsRunning)
	{
		if (WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
		{
			CrossServerNotifyStepFinished(StepIndex);
		}
		else
		{
			ServerNotifyStepFinished(StepIndex);
		}

		StopStepInternal();
	}
}

bool ASpatialFunctionalTestFlowController::IsLocalController() const
{
	ENetMode NetMode = GetNetMode();
	if (NetMode == NM_Standalone)
	{
		return true;
	}
	else if (GetLocalRole() == ROLE_Authority)
	{
		// if no PlayerController owns it it's ours.
		// @note keep in mind that this only works because each server worker has authority (by locking) their FlowController!
		return GetOwner() == nullptr;
	}
	else if (GetNetMode() == ENetMode::NM_Client)
	{
		// @note Clients only know their own PlayerController
		return GetOwner() != nullptr;
	}

	return false;
}

void ASpatialFunctionalTestFlowController::NotifyFinishTest(EFunctionalTestResult TestResult, const FString& Message)
{
	// To prevent trying to Notify multiple times, which can happen for example with multiple asserts in a row.
	if (CurrentStep.bIsRunning)
	{
		StopStepInternal();

		if (WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
		{
			ServerNotifyFinishTestInternal(TestResult, Message);
		}
		else
		{
			ServerNotifyFinishTest(TestResult, Message);
		}
	}
}

const FString ASpatialFunctionalTestFlowController::GetDisplayName() const
{
	return FString::Printf(TEXT("[%s:%d]"),
						   (WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server ? TEXT("Server") : TEXT("Client")),
						   WorkerDefinition.Id);
}

void ASpatialFunctionalTestFlowController::OnTestFinished()
{
	StopStepInternal();
	if (HasAuthority())
	{
		bHasAckFinishedTest = true;
	}
	else
	{
		ServerAckFinishedTest();
	}
	SetReadyToRunTest(false);
}

void ASpatialFunctionalTestFlowController::SetReadyToRunTest(bool bIsReady)
{
	if (bIsReady == bIsReadyToRunTest)
	{
		return;
	}
	if (IsLocalController())
	{
		if (WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
		{
			bIsReadyToRunTest = bIsReady;
		}
		else
		{
			ServerSetReadyToRunTest(bIsReady);
		}
	}
}

void ASpatialFunctionalTestFlowController::DeregisterFlowController()
{
	OwningTest->DeregisterFlowController(this);
}

void ASpatialFunctionalTestFlowController::ClientStartStep_Implementation(int StepIndex)
{
	OwningTest->SetCurrentStepIndex(StepIndex); // Just in case we do not get the replication fast enough
	StartStepInternal(StepIndex);
}

void ASpatialFunctionalTestFlowController::StartStepInternal(const int StepIndex)
{
	const FSpatialFunctionalTestStepDefinition& StepDefinition = OwningTest->GetStepDefinition(StepIndex);
	UE_LOG(LogSpatialGDKFunctionalTests, Log, TEXT("Executing step %s on %s"), *StepDefinition.StepName, *GetDisplayName());
	SetActorTickEnabled(true);
	CurrentStep.Start(StepDefinition);
}

void ASpatialFunctionalTestFlowController::StopStepInternal()
{
	CurrentStep.bIsRunning = false;
}

void ASpatialFunctionalTestFlowController::ServerNotifyFinishTest_Implementation(EFunctionalTestResult TestResult, const FString& Message)
{
	ServerNotifyFinishTestInternal(TestResult, Message);
}

void ASpatialFunctionalTestFlowController::ServerNotifyFinishTestInternal(EFunctionalTestResult TestResult, const FString& Message)
{
	OwningTest->CrossServerFinishTest(TestResult, Message);
}

void ASpatialFunctionalTestFlowController::ServerAckFinishedTest_Implementation()
{
	bHasAckFinishedTest = true;
}

void ASpatialFunctionalTestFlowController::ServerNotifyStepFinished_Implementation(const int StepIndex)
{
	OwningTest->CrossServerNotifyStepFinished(this, StepIndex);
}

void ASpatialFunctionalTestFlowController::CrossServerNotifyStepFinished_Implementation(const int StepIndex)
{
	OwningTest->CrossServerNotifyStepFinished(this, StepIndex);
}
