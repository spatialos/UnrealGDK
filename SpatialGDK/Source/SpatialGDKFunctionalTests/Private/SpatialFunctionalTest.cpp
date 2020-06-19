// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialFunctionalTest.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "SpatialFunctionalTestFlowController.h"

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
	checkf(AuxLocalFlowController != nullptr, TEXT("Can't Find LocalFlowController"));
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
		// Because of the way interest borders between server workers are setup, you can have clients being spawned outside the Test's
		// server worker, which means that it will have an id given locally by the server worker where the client spawned. So when registering,
		// the Test server worker will ensure that the clients all have unique ids. Note that server workers don't have this issue
		// because their ids are tied to their virtual worker id, hence guaranteed to be unique.
		uint8 DesiredClientId = FlowController->ControllerInstanceId;
		while (true)
		{
			bool bFoundConflictingId = false;
			for (auto* AuxController : FlowControllers)
			{
				if (AuxController->ControllerType == ESpatialFunctionalTestFlowControllerType::Client && AuxController->ControllerInstanceId == DesiredClientId)
				{
					bFoundConflictingId = true;
					DesiredClientId += 1; // there's a collision, let's try the next one
					checkf(DesiredClientId != 0, TEXT("This situation should only happen if you have more than 255 players, so you'll probably already be in trouble given that we don't support that"));
					break;

				}
			}
			if (!bFoundConflictingId)
			{
				if (FlowController->ControllerInstanceId != DesiredClientId)
				{
					FlowController->ControllerInstanceId = DesiredClientId;
					FlowController->CrossServerSetControllerInstanceId(DesiredClientId);
				}
				break;
			}
		}
	}
	
	FlowControllers.Add(FlowController);
}

ASpatialFunctionalTestFlowController* ASpatialFunctionalTest::GetLocalFlowController()
{
	checkf(LocalFlowController, TEXT("GetLocalFlowController being called without it being set, shouldn't happen, but falling back to finding it in World"));
	return LocalFlowController;
}

// Add Steps for Blueprints

void ASpatialFunctionalTest::AddUniversalStep(FString StepName, const FStepIsReadyDelegate& IsReadyEvent, const FStepStartDelegate& StartEvent, const FStepTickDelegate& TickEvent, float StepTimeLimit /*= 0.0f*/)
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

void ASpatialFunctionalTest::AddClientStep(FString StepName, int ClientId, const FStepIsReadyDelegate& IsReadyEvent, const FStepStartDelegate& StartEvent, const FStepTickDelegate& TickEvent, float StepTimeLimit /*= 0.0f*/)
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

void ASpatialFunctionalTest::AddServerStep(FString StepName, int ServerId, const FStepIsReadyDelegate& IsReadyEvent, const FStepStartDelegate& StartEvent, const FStepTickDelegate& TickEvent, float StepTimeLimit /*= 0.0f*/)
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

FSpatialFunctionalTestStepDefinition& ASpatialFunctionalTest::AddUniversalStep(FString StepName, FIsReadyEventFunc IsReadyEvent /*= nullptr*/, FStartEventFunc StartEvent /*= nullptr*/, FTickEventFunc TickEvent /*= nullptr*/, float StepTimeLimit /*= 0.0f*/)
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

FSpatialFunctionalTestStepDefinition& ASpatialFunctionalTest::AddClientStep(FString StepName, int ClientId, FIsReadyEventFunc IsReadyEvent /*= nullptr*/, FStartEventFunc StartEvent /*= nullptr*/, FTickEventFunc TickEvent /*= nullptr*/, float StepTimeLimit /*= 0.0f*/)
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

FSpatialFunctionalTestStepDefinition& ASpatialFunctionalTest::AddServerStep(FString StepName, int ServerId, FIsReadyEventFunc IsReadyEvent /*= nullptr*/, FStartEventFunc StartEvent /*= nullptr*/, FTickEventFunc TickEvent /*= nullptr*/, float StepTimeLimit /*= 0.0f*/)
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
		checkf(false, TEXT("%s"), *ErrorMsg);
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
