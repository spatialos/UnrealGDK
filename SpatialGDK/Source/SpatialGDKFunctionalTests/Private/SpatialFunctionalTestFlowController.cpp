// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialFunctionalTestFlowController.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "GameFramework/PlayerController.h"
#include "Interop/SpatialSender.h"
#include "LoadBalancing/LayeredLBStrategy.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTest.h"
#include "SpatialFunctionalTestGridLBStrategy.h"
#include "SpatialGDKFunctionalTestsPrivate.h"

#pragma optimize("", off)

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
}

void ASpatialFunctionalTestFlowController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialFunctionalTestFlowController, bReadyToRegisterWithTest);
	DOREPLIFETIME(ASpatialFunctionalTestFlowController, bIsReadyToRunTest);
	DOREPLIFETIME(ASpatialFunctionalTestFlowController, OwningTest);
	DOREPLIFETIME(ASpatialFunctionalTestFlowController, WorkerDefinition);
}

void ASpatialFunctionalTestFlowController::OnAuthorityGained()
{
	bReadyToRegisterWithTest = true;
	OnReadyToRegisterWithTest();
}

void ASpatialFunctionalTestFlowController::Tick(float DeltaSeconds)
{
	if (CurrentStep.bIsRunning)
	{
		CurrentStep.Tick(DeltaSeconds);
	}
}

void ASpatialFunctionalTestFlowController::CrossServerSetWorkerId_Implementation(int NewWorkerId)
{
	WorkerDefinition.Id = NewWorkerId;
}

void ASpatialFunctionalTestFlowController::AddEntityInterest_Implementation(const int64 ActorEntityId)
{
	ChangeEntityInterest(ActorEntityId, true);
}

void ASpatialFunctionalTestFlowController::RemoveEntityInterest_Implementation(const int64 ActorEntityId)
{
	ChangeEntityInterest(ActorEntityId, false);
}

void ASpatialFunctionalTestFlowController::ChangeEntityInterest(int64 ActorEntityId, bool bAddInterest)
{
	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetNetDriver());
	if (SpatialNetDriver == nullptr || ActorEntityId == SpatialConstants::INVALID_ENTITY_ID)
	{
		ensureMsgf(ActorEntityId != SpatialConstants::INVALID_ENTITY_ID, TEXT("Trying to change interest over an invalid entity id"));
		return;
	}

	ULayeredLBStrategy* LayeredLBStrategy = Cast<ULayeredLBStrategy>(SpatialNetDriver->LoadBalanceStrategy);
	USpatialFunctionalTestGridLBStrategy* TestGridLB =
		Cast<USpatialFunctionalTestGridLBStrategy>(LayeredLBStrategy->GetLBStrategyForVisualRendering());
	if (TestGridLB != nullptr)
	{
		int NumEntities = TestGridLB->Entities.Num();
		if (bAddInterest)
		{
			TestGridLB->Entities.AddUnique(ActorEntityId);
		}
		else
		{
			TestGridLB->Entities.Remove(ActorEntityId);
		}
		if (NumEntities != TestGridLB->Entities.Num())
		{
			SpatialNetDriver->Sender->UpdateServerWorkerEntityInterestAndPosition();
		}
	}
	else
	{
		UE_LOG(LogSpatialGDKFunctionalTests, Error, TEXT("Trying to change interest without USpatialFunctionalTestGridLBStrategy"));
	}
}

void ASpatialFunctionalTestFlowController::OnReadyToRegisterWithTest()
{
	if (!bReadyToRegisterWithTest || OwningTest == nullptr)
	{
		return;
	}

	OwningTest->RegisterFlowController(this);

	if (IsLocalController())
	{
		if (WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
		{
			bIsReadyToRunTest = true;
		}
		else
		{
			ServerSetReadyToRunTest();
		}
	}
}

void ASpatialFunctionalTestFlowController::ServerSetReadyToRunTest_Implementation()
{
	bIsReadyToRunTest = true;
}

void ASpatialFunctionalTestFlowController::CrossServerStartStep_Implementation(int StepIndex)
{
	if (WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
	{
		StartStepInternal(StepIndex);
	}
	else
	{
		ClientStartStep(StepIndex);
	}
}

void ASpatialFunctionalTestFlowController::NotifyStepFinished()
{
	if (CurrentStep.bIsRunning)
	{
		if (WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
		{
			CrossServerNotifyStepFinished();
		}
		else
		{
			ServerNotifyStepFinished();
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

const FString ASpatialFunctionalTestFlowController::GetDisplayName()
{
	return FString::Printf(TEXT("[%s:%d]"),
						   (WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server ? TEXT("Server") : TEXT("Client")),
						   WorkerDefinition.Id);
}

void ASpatialFunctionalTestFlowController::OnTestFinished()
{
	StopStepInternal();
}

void ASpatialFunctionalTestFlowController::ClientStartStep_Implementation(int StepIndex)
{
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
	SetActorTickEnabled(false);
	CurrentStep.Reset();
}

void ASpatialFunctionalTestFlowController::ServerNotifyFinishTest_Implementation(EFunctionalTestResult TestResult, const FString& Message)
{
	ServerNotifyFinishTestInternal(TestResult, Message);
}

void ASpatialFunctionalTestFlowController::ServerNotifyFinishTestInternal(EFunctionalTestResult TestResult, const FString& Message)
{
	OwningTest->CrossServerFinishTest(TestResult, Message);
}

void ASpatialFunctionalTestFlowController::ServerNotifyStepFinished_Implementation()
{
	OwningTest->CrossServerNotifyStepFinished(this);
}

void ASpatialFunctionalTestFlowController::CrossServerNotifyStepFinished_Implementation()
{
	OwningTest->CrossServerNotifyStepFinished(this);
}

#pragma optimize("", on)
