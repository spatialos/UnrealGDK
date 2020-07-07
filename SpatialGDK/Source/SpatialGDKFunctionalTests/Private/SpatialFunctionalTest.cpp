// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialFunctionalTest.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "SpatialFunctionalTestFlowController.h"
#include "LoadBalancing/LayeredLBStrategy.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpatialFunctionalTest, Log, All);

ASpatialFunctionalTest::ASpatialFunctionalTest()
	: Super()
	, FlowControllerSpawner(this, ASpatialFunctionalTestFlowController::StaticClass())
{
	bReplicates = true;
	NetPriority = 3.0f;
	NetUpdateFrequency = 100.0f;
	
	bAlwaysRelevant = true;
}

void ASpatialFunctionalTest::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASpatialFunctionalTest, bReadyToSpawnServerControllers);
	DOREPLIFETIME(ASpatialFunctionalTest, FlowControllers);
	DOREPLIFETIME(ASpatialFunctionalTest, CurrentStepIndex);
}

void ASpatialFunctionalTest::BeginPlay() 
{
	Super::BeginPlay();

	// by default expect 1 server
	NumExpectedServers = 1;

	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetNetDriver());
	if (SpatialNetDriver != nullptr) // if spatial enabled, determine how many should be there
	{
		UAbstractLBStrategy* LBStrategy = SpatialNetDriver->LoadBalanceStrategy;
		NumExpectedServers = LBStrategy != nullptr ? LBStrategy->GetMinimumRequiredWorkers() : 1;
	}

	if (GetWorld()->IsServer())
	{
		SetupClientPlayerRegistrationFlow();
	}
}

void ASpatialFunctionalTest::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Handle checking for finished
	if (CurrentStepIndex >= 0)
	{
		if (FlowControllersExecutingStep.Num() == 0)
		{
			int NextStepIndex = CurrentStepIndex + 1;
			if (NextStepIndex < StepDefinitions.Num())
			{
				StartStep(NextStepIndex);
			}
			else
			{
				FinishTest(EFunctionalTestResult::Succeeded, "");
			}
		}
		else 
		{
			TimeRunningStep += DeltaSeconds;

			float CurrentStepTimeLimit = StepDefinitions[CurrentStepIndex].TimeLimit;
						
			if (CurrentStepTimeLimit > 0.0f && TimeRunningStep >= CurrentStepTimeLimit)
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("Step time limit reached"));
			}
		}
	}
}

void ASpatialFunctionalTest::OnAuthorityGained()
{
	bReadyToSpawnServerControllers = true;
	StartServerFlowControllerSpawn();
}

void ASpatialFunctionalTest::RegisterAutoDestroyActor(AActor* ActorToAutoDestroy)
{
	if (HasAuthority())
	{
		Super::RegisterAutoDestroyActor(ActorToAutoDestroy);
	}
	else if(LocalFlowController != nullptr)
	{
		if(LocalFlowController->ControllerType == ESpatialFunctionalTestFlowControllerType::Server)
		{
			CrossServerRegisterAutoDestroyActor(ActorToAutoDestroy);
		}
		else
		{
			ServerRegisterAutoDestroyActor(ActorToAutoDestroy);
		}
	}
}

bool ASpatialFunctionalTest::IsReady_Implementation()
{
	int NumRegisteredClients = 0;
	int NumRegisteredServers = 0;

	for (ASpatialFunctionalTestFlowController* FlowController : FlowControllers)
	{
		if (FlowController->IsReadyToRunTest()) // Check if the owner already finished initialization
		{
			if (FlowController->ControllerType == ESpatialFunctionalTestFlowControllerType::Server)
			{
				++NumRegisteredServers;
			}
			else
			{
				++NumRegisteredClients;
			}
		}
	}

	checkf(NumRegisteredServers <= NumExpectedServers, TEXT("There's more servers registered than expected, this shouldn't happen"));

	return Super::IsReady_Implementation() && NumRegisteredClients >= NumRequiredClients && NumExpectedServers == NumRegisteredServers;
}

void ASpatialFunctionalTest::StartTest()
{
	Super::StartTest();

	StartStep(0);
}

void ASpatialFunctionalTest::FinishStep()
{
	auto* AuxLocalFlowController = GetLocalFlowController();
	ensureMsgf(AuxLocalFlowController != nullptr, TEXT("Can't Find LocalFlowController"));
	if(AuxLocalFlowController != nullptr)
	{
		AuxLocalFlowController->NotifyStepFinished();
	}
}

const FSpatialFunctionalTestStepDefinition ASpatialFunctionalTest::GetStepDefinition(int StepIndex) const
{
	if (StepIndex >= 0 && StepIndex < StepDefinitions.Num())
	{
		return StepDefinitions[StepIndex];
	}

	FSpatialFunctionalTestStepDefinition DummyStepDefinition;
	DummyStepDefinition.StepName = TEXT("No valid Step running, most likely still running a Step while the Test was aborted");
	return DummyStepDefinition;
}

int ASpatialFunctionalTest::GetNumberOfServerWorkers()
{
	int Counter = 0;
	for (ASpatialFunctionalTestFlowController* FlowController : FlowControllers)
	{
		if (FlowController != nullptr && FlowController->ControllerType == ESpatialFunctionalTestFlowControllerType::Server)
		{
			++Counter;
		}
	}
	return Counter;
}

int ASpatialFunctionalTest::GetNumberOfClientWorkers()
{
	int Counter = 0;
	for (ASpatialFunctionalTestFlowController* FlowController : FlowControllers)
	{
		if (FlowController != nullptr && FlowController->ControllerType == ESpatialFunctionalTestFlowControllerType::Client)
		{
			++Counter;
		}
	}
	return Counter;
}

void ASpatialFunctionalTest::AddActorDelegation_Implementation(AActor* Actor, uint8 ServerWorkerId, bool bPersistOnTestFinished /*= false*/)
{
	ISpatialFunctionalTestLBDelegationInterface* DelegationInterface = GetDelegationInterface();

	if (DelegationInterface != NULL)
	{
		bool bAddedDelegation = DelegationInterface->AddActorDelegation(Actor, ServerWorkerId, bPersistOnTestFinished);
		ensureMsgf(bAddedDelegation, TEXT("Tried to delegate Actor %s to Server Worker %d but couldn't"), *Actor->GetName(), ServerWorkerId);
	}
}

void ASpatialFunctionalTest::RemoveActorDelegation_Implementation(AActor* Actor)
{
	ISpatialFunctionalTestLBDelegationInterface* DelegationInterface = GetDelegationInterface();

	if (DelegationInterface != nullptr)
	{
		bool bRemovedDelegation = DelegationInterface->RemoveActorDelegation(Actor);
		ensureMsgf(bRemovedDelegation, TEXT("Tried to remove Delegation from Actor %s but couldn't"), *Actor->GetName());
	}
}

bool ASpatialFunctionalTest::HasActorDelegation(AActor* Actor, uint8& WorkerId, bool& bIsPersistent)
{
	WorkerId = 0;
	bIsPersistent = 0;

	ISpatialFunctionalTestLBDelegationInterface* DelegationInterface = GetDelegationInterface();

	bool bHasDelegation = false;

	if (DelegationInterface != nullptr)
	{
		VirtualWorkerId AuxWorkerId;

		bHasDelegation = DelegationInterface->HasActorDelegation(Actor, AuxWorkerId, bIsPersistent);

		WorkerId = static_cast<uint8>(AuxWorkerId);
	}

	return bHasDelegation;
}

ISpatialFunctionalTestLBDelegationInterface* ASpatialFunctionalTest::GetDelegationInterface() const
{
	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetNetDriver());
	if (SpatialNetDriver)
	{
		ULayeredLBStrategy* LayeredLBStrategy = Cast<ULayeredLBStrategy>(SpatialNetDriver->LoadBalanceStrategy);
		if(LayeredLBStrategy != nullptr)
		{
			return Cast<ISpatialFunctionalTestLBDelegationInterface>(LayeredLBStrategy->GetLBStrategyForVisualRendering());
		}
	}
	return nullptr;
}

void ASpatialFunctionalTest::FinishTest(EFunctionalTestResult TestResult, const FString& Message)
{
	if (HasAuthority())
	{
		UE_LOG(LogSpatialFunctionalTest, Display, TEXT("Test %s finished! Result: %s ; Message: %s"), *GetName(), *UEnum::GetValueAsString(TestResult), *Message);

		CurrentStepIndex = SPATIAL_FUNCTIONAL_TEST_FINISHED;
		OnReplicated_CurrentStepIndex(); // need to call it in Authority manually
		MulticastAutoDestroyActors(AutoDestroyActors);

		Super::FinishTest(TestResult, Message);
	}
	else
	{
		ASpatialFunctionalTestFlowController* AuxLocalFlowController = GetLocalFlowController();
		if (AuxLocalFlowController != nullptr)
		{
			AuxLocalFlowController->NotifyFinishTest(TestResult, Message);
		}
	}
}

void ASpatialFunctionalTest::CrossServerFinishTest_Implementation(EFunctionalTestResult TestResult, const FString& Message)
{
	FinishTest(TestResult, Message);
}

void ASpatialFunctionalTest::RegisterFlowController(ASpatialFunctionalTestFlowController* FlowController)
{
	if (FlowController->IsLocalController())
	{
		checkf(LocalFlowController == nullptr, TEXT("Already had LocalFlowController, this shouldn't happen"));
		LocalFlowController = FlowController;
	}

	if (!HasAuthority())
	{
		//FlowControllers invoke this on each worker's local context when checkout and ready, we only want to act in the authority
		return;
	}

	if (FlowController->ControllerType == ESpatialFunctionalTestFlowControllerType::Client)
	{
		// Since Clients can spawn on any worker we need to centralize the assignment of their ids to the Test Authority.
		FlowControllerSpawner.AssignClientFlowControllerId(FlowController);
	}
	
	FlowControllers.Add(FlowController);
}

ASpatialFunctionalTestFlowController* ASpatialFunctionalTest::GetLocalFlowController()
{
	ensureMsgf(LocalFlowController, TEXT("GetLocalFlowController being called without it being set, shouldn't happen"));
	return LocalFlowController;
}

// Add Steps for Blueprints

void ASpatialFunctionalTest::AddUniversalStep(const FString& StepName, const FStepIsReadyDelegate& IsReadyEvent, const FStepStartDelegate& StartEvent, const FStepTickDelegate& TickEvent, float StepTimeLimit /*= 0.0f*/)
{
	FSpatialFunctionalTestStepDefinition StepDefinition;
	StepDefinition.bIsNativeDefinition = false;
	StepDefinition.StepName = StepName;
	StepDefinition.IsReadyEvent = IsReadyEvent;
	StepDefinition.StartEvent = StartEvent;
	StepDefinition.TickEvent = TickEvent;
	StepDefinition.TimeLimit = StepTimeLimit;

	StepDefinition.Workers.Add(FWorkerDefinition{ ESpatialFunctionalTestFlowControllerType::Server, FWorkerDefinition::ALL_WORKERS_ID });
	StepDefinition.Workers.Add(FWorkerDefinition{ ESpatialFunctionalTestFlowControllerType::Client, FWorkerDefinition::ALL_WORKERS_ID });

	StepDefinitions.Add(StepDefinition);
}

void ASpatialFunctionalTest::AddClientStep(const FString& StepName, int ClientId, const FStepIsReadyDelegate& IsReadyEvent, const FStepStartDelegate& StartEvent, const FStepTickDelegate& TickEvent, float StepTimeLimit /*= 0.0f*/)
{
	FSpatialFunctionalTestStepDefinition StepDefinition;
	StepDefinition.bIsNativeDefinition = false;
	StepDefinition.StepName = StepName;
	StepDefinition.IsReadyEvent = IsReadyEvent;
	StepDefinition.StartEvent = StartEvent;
	StepDefinition.TickEvent = TickEvent;
	StepDefinition.TimeLimit = StepTimeLimit;

	StepDefinition.Workers.Add(FWorkerDefinition{ ESpatialFunctionalTestFlowControllerType::Client, ClientId });

	StepDefinitions.Add(StepDefinition);
}

void ASpatialFunctionalTest::AddServerStep(const FString& StepName, int ServerId, const FStepIsReadyDelegate& IsReadyEvent, const FStepStartDelegate& StartEvent, const FStepTickDelegate& TickEvent, float StepTimeLimit /*= 0.0f*/)
{
	FSpatialFunctionalTestStepDefinition StepDefinition;
	StepDefinition.bIsNativeDefinition = false;
	StepDefinition.StepName = StepName;
	StepDefinition.IsReadyEvent = IsReadyEvent;
	StepDefinition.StartEvent = StartEvent;
	StepDefinition.TickEvent = TickEvent;
	StepDefinition.TimeLimit = StepTimeLimit;

	StepDefinition.Workers.Add(FWorkerDefinition{ ESpatialFunctionalTestFlowControllerType::Server, ServerId });

	StepDefinitions.Add(StepDefinition);
}

void ASpatialFunctionalTest::AddGenericStep(const FSpatialFunctionalTestStepDefinition& StepDefinition)
{
	StepDefinitions.Add(StepDefinition);
}

void ASpatialFunctionalTest::StartStep(const int StepIndex)
{
	if (HasAuthority())
	{
		CurrentStepIndex = StepIndex;

		TimeRunningStep = 0.0f;

		FSpatialFunctionalTestStepDefinition& StepDefinition = StepDefinitions[CurrentStepIndex];

		for (const FWorkerDefinition& Worker : StepDefinition.Workers)
		{
			int WorkerId = Worker.WorkerId;
			if (NumExpectedServers == 1 && Worker.ControllerType == ESpatialFunctionalTestFlowControllerType::Server)
			{
				// make sure that tests made for multi server also run on single server
				WorkerId = 1;
			}
			for (auto* FlowController : FlowControllers)
			{
				if (FlowController->ControllerType == Worker.ControllerType &&
					(WorkerId <= FWorkerDefinition::ALL_WORKERS_ID || FlowController->ControllerInstanceId == WorkerId))
				{
					FlowControllersExecutingStep.AddUnique(FlowController);
				}
			}
		}

		if (FlowControllersExecutingStep.Num() > 0)
		{
			bIsRunning = true;
			FString Msg = FString::Printf(TEXT(">> Starting Step %s on: "), *StepDefinition.StepName);
			for (int i = FlowControllersExecutingStep.Num() - 1; i >= 0; --i)
			{
				ASpatialFunctionalTestFlowController* FlowController = FlowControllersExecutingStep[i];

				Msg.Append(FlowController->GetDisplayName());

				if (i > 0)
				{
					Msg.Append(TEXT(", "));
				}

				FlowController->CrossServerStartStep(CurrentStepIndex);
			}

			UE_LOG(LogSpatialFunctionalTest, Display, TEXT("%s"), *Msg);
		}
		else
		{
			FinishTest(EFunctionalTestResult::Error, FString::Printf(TEXT("Trying to start Step %s without any worker"), *StepDefinition.StepName));
		}
	}
}

// Add Steps for C++

FSpatialFunctionalTestStepDefinition& ASpatialFunctionalTest::AddUniversalStep(const FString& StepName, FIsReadyEventFunc IsReadyEvent /*= nullptr*/, FStartEventFunc StartEvent /*= nullptr*/, FTickEventFunc TickEvent /*= nullptr*/, float StepTimeLimit /*= 0.0f*/)
{
	FSpatialFunctionalTestStepDefinition StepDefinition;
	StepDefinition.bIsNativeDefinition = true;
	StepDefinition.StepName = StepName;
	if (IsReadyEvent)
	{
		StepDefinition.NativeIsReadyEvent.BindLambda(IsReadyEvent);
	}
	if (StartEvent)
	{
		StepDefinition.NativeStartEvent.BindLambda(StartEvent);
	}
	if (TickEvent)
	{
		StepDefinition.NativeTickEvent.BindLambda(TickEvent);
	}
	StepDefinition.TimeLimit = StepTimeLimit;

	StepDefinition.Workers.Add(FWorkerDefinition{ ESpatialFunctionalTestFlowControllerType::Server, FWorkerDefinition::ALL_WORKERS_ID });
	StepDefinition.Workers.Add(FWorkerDefinition{ ESpatialFunctionalTestFlowControllerType::Client, FWorkerDefinition::ALL_WORKERS_ID });

	StepDefinitions.Add(StepDefinition);

	return StepDefinitions[StepDefinitions.Num() - 1];
}

FSpatialFunctionalTestStepDefinition& ASpatialFunctionalTest::AddClientStep(const FString& StepName, int ClientId, FIsReadyEventFunc IsReadyEvent /*= nullptr*/, FStartEventFunc StartEvent /*= nullptr*/, FTickEventFunc TickEvent /*= nullptr*/, float StepTimeLimit /*= 0.0f*/)
{
	FSpatialFunctionalTestStepDefinition StepDefinition;
	StepDefinition.bIsNativeDefinition = true;
	StepDefinition.StepName = StepName;
	if (IsReadyEvent)
	{
		StepDefinition.NativeIsReadyEvent.BindLambda(IsReadyEvent);
	}
	if (StartEvent)
	{
		StepDefinition.NativeStartEvent.BindLambda(StartEvent);
	}
	if (TickEvent)
	{
		StepDefinition.NativeTickEvent.BindLambda(TickEvent);
	}
	StepDefinition.TimeLimit = StepTimeLimit;

	StepDefinition.Workers.Add(FWorkerDefinition{ ESpatialFunctionalTestFlowControllerType::Client, ClientId });

	StepDefinitions.Add(StepDefinition);

	return StepDefinitions[StepDefinitions.Num() - 1];
}

FSpatialFunctionalTestStepDefinition& ASpatialFunctionalTest::AddServerStep(const FString& StepName, int ServerId, FIsReadyEventFunc IsReadyEvent /*= nullptr*/, FStartEventFunc StartEvent /*= nullptr*/, FTickEventFunc TickEvent /*= nullptr*/, float StepTimeLimit /*= 0.0f*/)
{
	FSpatialFunctionalTestStepDefinition StepDefinition;
	StepDefinition.bIsNativeDefinition = true;
	StepDefinition.StepName = StepName;
	if (IsReadyEvent)
	{
		StepDefinition.NativeIsReadyEvent.BindLambda(IsReadyEvent);
	}
	if (StartEvent)
	{
		StepDefinition.NativeStartEvent.BindLambda(StartEvent);
	}
	if (TickEvent)
	{
		StepDefinition.NativeTickEvent.BindLambda(TickEvent);
	}
	StepDefinition.TimeLimit = StepTimeLimit;

	StepDefinition.Workers.Add(FWorkerDefinition{ ESpatialFunctionalTestFlowControllerType::Server, ServerId });

	StepDefinitions.Add(StepDefinition);

	return StepDefinitions[StepDefinitions.Num() - 1];
}


ASpatialFunctionalTestFlowController* ASpatialFunctionalTest::GetFlowController(ESpatialFunctionalTestFlowControllerType ControllerType, int InstanceId)
{
	for (auto* FlowController : FlowControllers)
	{
		if (FlowController->ControllerType == ControllerType && FlowController->ControllerInstanceId == InstanceId)
		{
			return FlowController;
		}
	}
	return nullptr;
}

void ASpatialFunctionalTest::CrossServerNotifyStepFinished_Implementation(ASpatialFunctionalTestFlowController* FlowController)
{
	if (CurrentStepIndex < 0)
	{
		return;
	}

	const FString FLowControllerDisplayName = FlowController->GetDisplayName();
	
	UE_LOG(LogSpatialFunctionalTest, Display, TEXT("%s finished Step"), *FLowControllerDisplayName);
	
	if (FlowControllersExecutingStep.RemoveSwap(FlowController) == 0)
	{
		FString ErrorMsg = FString::Printf(TEXT("%s was not in list of workers executing"), *FLowControllerDisplayName);
		ensureMsgf(false, TEXT("%s"), *ErrorMsg);
		FinishTest(EFunctionalTestResult::Error, ErrorMsg);
	}
}

void ASpatialFunctionalTest::OnReplicated_CurrentStepIndex()
{
	if (CurrentStepIndex == SPATIAL_FUNCTIONAL_TEST_FINISHED)
	{
		//test finished
		if(StartTime > 0)
		{
			//if we ever started in first place
			ASpatialFunctionalTestFlowController* AuxLocalFlowController = GetLocalFlowController();
			if (AuxLocalFlowController != nullptr)
			{
				AuxLocalFlowController->OnTestFinished();
			}
		}
		if (!HasAuthority()) // Authority already does this on Super::FinishTest
		{
			NotifyTestFinishedObserver();
		}
	}
}

void ASpatialFunctionalTest::StartServerFlowControllerSpawn()
{
	if (!bReadyToSpawnServerControllers)
	{
		return;
	}

	if (FlowControllerActorClass.Get() != nullptr)
	{
		FlowControllerSpawner.ModifyFlowControllerClassToSpawn(FlowControllerActorClass);
	}

	FlowControllerSpawner.SpawnServerFlowController();
}

void ASpatialFunctionalTest::SetupClientPlayerRegistrationFlow()
{
	GetWorld()->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateLambda(
		[this](AActor* Spawned)
		{
			if (APlayerController* PlayerController = Cast<APlayerController>(Spawned))
			{
				if(PlayerController->HasAuthority())
				{
					this->FlowControllerSpawner.SpawnClientFlowController(PlayerController);
				}
			}
		}
	));
}

void ASpatialFunctionalTest::CrossServerRegisterAutoDestroyActor_Implementation(AActor* ActorToAutoDestroy)
{
	RegisterAutoDestroyActor(ActorToAutoDestroy);
}

void ASpatialFunctionalTest::ServerRegisterAutoDestroyActor_Implementation(AActor* ActorToAutoDestroy)
{
	CrossServerRegisterAutoDestroyActor(ActorToAutoDestroy);
}

void ASpatialFunctionalTest::MulticastAutoDestroyActors_Implementation(const TArray<AActor*>& ActorsToDestroy)
{
	FString DisplayName = LocalFlowController ? LocalFlowController->GetDisplayName() : TEXT("UNKNOWN");
	if (!HasAuthority()) // Authority already handles it in Super::FinishTest
	{
		for (AActor* Actor : ActorsToDestroy)
		{
			if (IsValid(Actor))
			{				
				UE_LOG(LogSpatialFunctionalTest, Display, TEXT("%s trying to delete actor: %s ; result now would be: %s"), *DisplayName, *Actor->GetName(), Actor->Role == ROLE_Authority ? TEXT("SUCCESS") : TEXT("FAILURE"));
				Actor->SetLifeSpan(0.01f);
			}
		}
	}
	else
	{
		for (AActor* Actor : ActorsToDestroy)
		{
			if (IsValid(Actor))
			{
				UE_LOG(LogSpatialFunctionalTest, Display, TEXT("%s TEST_AUTH - will have tried to delete actor: %s ; result now would be: %s"), *DisplayName, *Actor->GetName(), Actor->Role == ROLE_Authority ? TEXT("SUCCESS") : TEXT("FAILURE"));
			}
		}
	}
}
