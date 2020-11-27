// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialFunctionalTest.h"

#include "AutomationBlueprintFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialNetDriverDebugContext.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManagerGeneric.h"
#include "HttpModule.h"
#include "Improbable/SpatialGDKSettingsBridge.h"
#include "Interfaces/IHttpBase.h"
#include "Interfaces/IHttpResponse.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "LoadBalancing/DebugLBStrategy.h"
#include "LoadBalancing/LayeredLBStrategy.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTestAutoDestroyComponent.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTestsPrivate.h"

namespace
{
// Maximum time that the test authority will wait after deciding to FinishTest in order for all the Workers
// to have enough time to acknowledge it. If this time is exceeded, the test authority will force FinishTest.
constexpr float FINISH_TEST_GRACE_PERIOD_DURATION = 2.0f;
} // namespace

ASpatialFunctionalTest::ASpatialFunctionalTest()
	: Super()
	, FlowControllerSpawner(this, ASpatialFunctionalTestFlowController::StaticClass())
{
	bReplicates = true;
	NetPriority = 3.0f;
	NetUpdateFrequency = 100.0f;

	bAlwaysRelevant = true;

	PrimaryActorTick.TickInterval = 0.0f;

	PreparationTimeLimit = 30.0f;
}

void ASpatialFunctionalTest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASpatialFunctionalTest, bReadyToSpawnServerControllers);
	DOREPLIFETIME(ASpatialFunctionalTest, FlowControllers);
	DOREPLIFETIME(ASpatialFunctionalTest, CurrentStepIndex);
	DOREPLIFETIME(ASpatialFunctionalTest, bPreparedTest);
}

void ASpatialFunctionalTest::BeginPlay()
{
	Super::BeginPlay();

	// Setup built-in step definitions.
	TakeSnapshotStepDefinition = FSpatialFunctionalTestStepDefinition(true);
	TakeSnapshotStepDefinition.StepName = TEXT("Take SpatialOS Snapshot");
	TakeSnapshotStepDefinition.NativeStartEvent.BindLambda([this]() {
		TakeSnapshot([this](bool bSuccess) {
			if (bSuccess)
			{
				FinishStep();
			}
			else
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("Failed to take SpatialOS Snapshot."));
			}
		});
	});

	ClearSnapshotStepDefinition = FSpatialFunctionalTestStepDefinition(true);
	ClearSnapshotStepDefinition.StepName = TEXT("Clear SpatialOS Snapshot");
	ClearSnapshotStepDefinition.NativeStartEvent.BindLambda([this]() {
		ClearSnapshot();
		FinishStep();
	});

	RequireHandler.SetOwnerTest(this);

	// By default expect 1 server.
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
	else if (CurrentStepIndex == SPATIAL_FUNCTIONAL_TEST_FINISHED && FinishTestTimerHandle.IsValid())
	{
		bool bAllAcknowledgedFinishedTest = true;
		for (const auto* FlowController : FlowControllers)
		{
			if (!FlowController->HasAckFinishedTest())
			{
				bAllAcknowledgedFinishedTest = false;
				break;
			}
		}
		if (bAllAcknowledgedFinishedTest)
		{
			GetWorld()->GetTimerManager().ClearTimer(FinishTestTimerHandle);
			Super::FinishTest(CachedTestResult, CachedTestMessage);
			// Clear cached variables
			CachedTestResult = EFunctionalTestResult::Default;
			CachedTestMessage.Empty();
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
	if (ActorToAutoDestroy != nullptr && ActorToAutoDestroy->HasAuthority())
	{
		// Add component to actor to auto destroy when test finishes
		USpatialFunctionalTestAutoDestroyComponent* AutoDestroyComponent =
			NewObject<USpatialFunctionalTestAutoDestroyComponent>(ActorToAutoDestroy);
		AutoDestroyComponent->AttachToComponent(ActorToAutoDestroy->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		AutoDestroyComponent->RegisterComponent();
	}
	else
	{
		UE_LOG(LogSpatialGDKFunctionalTests, Error,
			   TEXT("Should only register to auto destroy from the authoritative worker of the actor: %s"),
			   *GetNameSafe(ActorToAutoDestroy));
	}
}

void ASpatialFunctionalTest::LogStep(ELogVerbosity::Type Verbosity, const FString& Message)
{
	Super::LogStep(Verbosity, Message);

	if (Verbosity == ELogVerbosity::Error || Verbosity == ELogVerbosity::Fatal)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Failed assertions"));
	}
}

void ASpatialFunctionalTest::PrepareTest()
{
	StepDefinitions.Empty();

	Super::PrepareTest();

	if (HasAuthority())
	{
		bPreparedTest = true;
		OnReplicated_bPreparedTest();
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
			if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
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
	// We can only FinishStep if there are no Require fails.
	if (RequireHandler.HasFails())
	{
		return;
	}

	RequireHandler.LogAndClearStepRequires();

	auto* AuxLocalFlowController = GetLocalFlowController();
	ensureMsgf(AuxLocalFlowController != nullptr, TEXT("Can't Find LocalFlowController"));
	if (AuxLocalFlowController != nullptr)
	{
		AuxLocalFlowController->NotifyStepFinished(CurrentStepIndex);
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
		if (FlowController != nullptr && FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
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
		if (FlowController != nullptr && FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Client)
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
		// Make sure we don't FinishTest multiple times.
		if (CurrentStepIndex != SPATIAL_FUNCTIONAL_TEST_FINISHED)
		{
			bPreparedTest = false; // Clear for PrepareTest to run on all again if the test re-runs.
			OnReplicated_bPreparedTest();

			UE_LOG(LogSpatialGDKFunctionalTests, Display, TEXT("Test %s finished! Result: %s ; Message: %s"), *GetName(),
				   *UEnum::GetValueAsString(TestResult), *Message);

			if (TestResult == TimesUpResult)
			{
				int NumRegisteredClients = 0;
				int NumRegisteredServers = 0;

				for (ASpatialFunctionalTestFlowController* FlowController : FlowControllers)
				{
					if (FlowController->IsReadyToRunTest()) // Check if the owner already finished initialization
					{
						if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
						{
							++NumRegisteredServers;
						}
						else
						{
							++NumRegisteredClients;
						}
					}
				}

				if (NumRegisteredClients < NumRequiredClients)
				{
					UE_LOG(
						LogSpatialGDKFunctionalTests, Warning,
						TEXT("In %s, the number of connected clients is less than the number of required clients: Connected clients: %d, "
							 "Required clients: %d!"),
						*GetName(), NumRegisteredClients, NumRequiredClients);
				}

				if (NumRegisteredServers < NumExpectedServers)
				{
					UE_LOG(
						LogSpatialGDKFunctionalTests, Warning,
						TEXT("In %s, the number of connected servers is less than the number of required servers: Connected servers: %d, "
							 "Required servers: %d!"),
						*GetName(), NumRegisteredServers, NumExpectedServers);
				}
			}

			CurrentStepIndex = SPATIAL_FUNCTIONAL_TEST_FINISHED;
			OnReplicated_CurrentStepIndex(); // need to call it in Authority manually

			CachedTestResult = TestResult;
			CachedTestMessage = Message;

			GetWorld()->GetTimerManager().SetTimer(
				FinishTestTimerHandle,
				[this]() {
					// If this timer triggers, then it means that something went wrong with one of the Workers. The
					// expected behaviour is that the Super::FinishTest will be called from Tick(). Let's double check
					// which failed to reply.
					FString WorkersDidntAck;
					for (const auto* FlowController : FlowControllers)
					{
						if (!FlowController->HasAckFinishedTest())
						{
							WorkersDidntAck += FString::Printf(TEXT("%s, "), *(FlowController->GetDisplayName()));
						}
					}
					if (!WorkersDidntAck.IsEmpty())
					{
						WorkersDidntAck.RemoveFromEnd(TEXT(", "));
						UE_LOG(LogSpatialGDKFunctionalTests, Warning,
							   TEXT("The following Workers failed to acknowledge FinishTest in time: %s"), *WorkersDidntAck);
					}

					Super::FinishTest(CachedTestResult, CachedTestMessage);

					FinishTestTimerHandle.Invalidate();

					// Clear cached values.
					CachedTestResult = EFunctionalTestResult::Default;
					CachedTestMessage.Empty();
				},
				FINISH_TEST_GRACE_PERIOD_DURATION, false /* InbLoop */);
		}
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

void ASpatialFunctionalTest::AddExpectedLogError(const FString& ExpectedPatternString, int32 Occurrences /*= 1*/,
												 bool bExactMatch /*= false*/)
{
	UAutomationBlueprintFunctionLibrary::AddExpectedLogError(ExpectedPatternString, Occurrences, bExactMatch);
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
		// FlowControllers invoke this on each worker's local context when checkout and ready, we only want to act in the authority
		return;
	}

	if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Client)
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

ESpatialFunctionalTestWorkerType ASpatialFunctionalTest::GetLocalWorkerType()
{
	ASpatialFunctionalTestFlowController* AuxFlowController = GetLocalFlowController();
	return AuxFlowController != nullptr ? AuxFlowController->WorkerDefinition.Type : ESpatialFunctionalTestWorkerType::Invalid;
}

int ASpatialFunctionalTest::GetLocalWorkerId()
{
	ASpatialFunctionalTestFlowController* AuxFlowController = GetLocalFlowController();
	return AuxFlowController != nullptr ? AuxFlowController->WorkerDefinition.Id : INVALID_FLOW_CONTROLLER_ID;
}

// Add Steps for Blueprints

void ASpatialFunctionalTest::AddStepBlueprint(const FString& StepName, const FWorkerDefinition& Worker,
											  const FStepIsReadyDelegate& IsReadyEvent, const FStepStartDelegate& StartEvent,
											  const FStepTickDelegate& TickEvent, float StepTimeLimit /*= 0.0f*/)
{
	if (StepName.IsEmpty())
	{
		UE_LOG(LogSpatialGDKFunctionalTests, Warning, TEXT("Adding a Step without a name"));
	}

	FSpatialFunctionalTestStepDefinition StepDefinition;
	StepDefinition.bIsNativeDefinition = false;
	StepDefinition.StepName = StepName;
	StepDefinition.IsReadyEvent = IsReadyEvent;
	StepDefinition.StartEvent = StartEvent;
	StepDefinition.TickEvent = TickEvent;
	StepDefinition.TimeLimit = StepTimeLimit;

	StepDefinition.Workers.Add(Worker);

	StepDefinitions.Add(StepDefinition);
}

void ASpatialFunctionalTest::AddStepFromDefinition(const FSpatialFunctionalTestStepDefinition& StepDefinition,
												   const FWorkerDefinition& Worker)
{
	if (StepDefinition.StepName.IsEmpty())
	{
		UE_LOG(LogSpatialGDKFunctionalTests, Warning, TEXT("Adding a Step without a name"));
	}
	FSpatialFunctionalTestStepDefinition StepDefinitionCopy = StepDefinition;

	StepDefinitionCopy.Workers.Add(Worker);

	StepDefinitions.Add(StepDefinitionCopy);
}

void ASpatialFunctionalTest::AddStepFromDefinitionMulti(const FSpatialFunctionalTestStepDefinition& StepDefinition,
														const TArray<FWorkerDefinition>& Workers)
{
	if (StepDefinition.StepName.IsEmpty())
	{
		UE_LOG(LogSpatialGDKFunctionalTests, Warning, TEXT("Adding a Step without a name"));
	}
	FSpatialFunctionalTestStepDefinition StepDefinitionCopy = StepDefinition;

	StepDefinitionCopy.Workers.Append(Workers);

	StepDefinitions.Add(StepDefinitionCopy);
}

void ASpatialFunctionalTest::StartStep(const int StepIndex)
{
	if (HasAuthority())
	{
		// Log Requires from previous step.
		RequireHandler.LogAndClearStepRequires();

		CurrentStepIndex = StepIndex;

		TimeRunningStep = 0.0f;

		FSpatialFunctionalTestStepDefinition& StepDefinition = StepDefinitions[CurrentStepIndex];

		for (const FWorkerDefinition& Worker : StepDefinition.Workers)
		{
			ESpatialFunctionalTestWorkerType WorkerType = Worker.Type;
			int WorkerId = Worker.Id;
			if (NumExpectedServers == 1 && WorkerType == ESpatialFunctionalTestWorkerType::Server)
			{
				// make sure that tests made for multi server also run on single server
				WorkerId = 1;
			}
			for (auto* FlowController : FlowControllers)
			{
				if (WorkerType == ESpatialFunctionalTestWorkerType::All
					|| (FlowController->WorkerDefinition.Type == WorkerType
						&& (WorkerId <= FWorkerDefinition::ALL_WORKERS_ID || FlowController->WorkerDefinition.Id == WorkerId)))
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

			UE_LOG(LogSpatialGDKFunctionalTests, Display, TEXT("%s"), *Msg);
		}
		else
		{
			FinishTest(EFunctionalTestResult::Error,
					   FString::Printf(TEXT("Trying to start Step %s without any Worker"), *StepDefinition.StepName));
		}
	}
}

// Add Steps for C++

FSpatialFunctionalTestStepDefinition& ASpatialFunctionalTest::AddStep(const FString& StepName, const FWorkerDefinition& Worker,
																	  FIsReadyEventFunc IsReadyEvent /*= nullptr*/,
																	  FStartEventFunc StartEvent /*= nullptr*/,
																	  FTickEventFunc TickEvent /*= nullptr*/,
																	  float StepTimeLimit /*= 0.0f*/)
{
	if (StepName.IsEmpty())
	{
		UE_LOG(LogSpatialGDKFunctionalTests, Warning, TEXT("Adding a Step without a name"));
	}

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

	StepDefinition.Workers.Add(Worker);

	StepDefinitions.Add(StepDefinition);

	return StepDefinitions[StepDefinitions.Num() - 1];
}

ASpatialFunctionalTestFlowController* ASpatialFunctionalTest::GetFlowController(ESpatialFunctionalTestWorkerType WorkerType, int WorkerId)
{
	ensureMsgf(WorkerType != ESpatialFunctionalTestWorkerType::All, TEXT("Trying to call GetFlowController with All WorkerType"));
	for (auto* FlowController : FlowControllers)
	{
		if (FlowController->WorkerDefinition.Type == WorkerType && FlowController->WorkerDefinition.Id == WorkerId)
		{
			return FlowController;
		}
	}
	return nullptr;
}

void ASpatialFunctionalTest::CrossServerNotifyStepFinished_Implementation(ASpatialFunctionalTestFlowController* FlowController,
																		  const int StepIndex)
{
	if (CurrentStepIndex < 0 || StepIndex != CurrentStepIndex)
	{
		return;
	}

	const FString FlowControllerDisplayName = FlowController->GetDisplayName();

	UE_LOG(LogSpatialGDKFunctionalTests, Display, TEXT("%s finished Step"), *FlowControllerDisplayName);

	if (FlowControllersExecutingStep.RemoveSwap(FlowController) == 0)
	{
		FString ErrorMsg = FString::Printf(TEXT("%s was not in list of workers executing"), *FlowControllerDisplayName);
		ensureMsgf(false, TEXT("%s"), *ErrorMsg);
		FinishTest(EFunctionalTestResult::Error, ErrorMsg);
	}
}

void ASpatialFunctionalTest::OnReplicated_CurrentStepIndex()
{
	if (CurrentStepIndex == SPATIAL_FUNCTIONAL_TEST_FINISHED)
	{
		RequireHandler.LogAndClearStepRequires();
		// if we ever started in first place
		ASpatialFunctionalTestFlowController* AuxLocalFlowController = GetLocalFlowController();
		if (AuxLocalFlowController != nullptr)
		{
			AuxLocalFlowController->OnTestFinished();
			if (AuxLocalFlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
			{
				ClearTagDelegationAndInterest();
			}
		}
		if (!HasAuthority()) // Authority already does this on Super::FinishTest
		{
			NotifyTestFinishedObserver();
		}

		DeleteActorsRegisteredForAutoDestroy();
	}
}

void ASpatialFunctionalTest::OnReplicated_bPreparedTest()
{
	if (bPreparedTest)
	{
		// We need to delay until next Tick since on non-Authority
		// OnReplicated_bPreparedTest() will be called before BeginPlay().
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]() {
			if (!HasAuthority())
			{
				PrepareTest();
			}

			// Currently PrepareTest() happens before FlowControllers are registered,
			// but that is most likely because of the bug that forces us to delay their registration.
			if (LocalFlowController != nullptr)
			{
				LocalFlowController->SetReadyToRunTest(true);
			}
		});
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
	PostLoginDelegate = FGameModeEvents::GameModePostLoginEvent.AddLambda([this](AGameModeBase* GameMode, APlayerController* NewPlayer) {
		// NB : the delegate is a global one, have to filter in case we are running from PIE <==> multiple worlds.
		if (NewPlayer->GetWorld() == GetWorld() && NewPlayer->HasAuthority())
		{
			this->FlowControllerSpawner.SpawnClientFlowController(NewPlayer);
		}
	});
}

void ASpatialFunctionalTest::EndPlay(const EEndPlayReason::Type Reason)
{
	if (PostLoginDelegate.IsValid())
	{
		FGameModeEvents::GameModePostLoginEvent.Remove(PostLoginDelegate);
		PostLoginDelegate.Reset();
	}
}

void ASpatialFunctionalTest::DeleteActorsRegisteredForAutoDestroy()
{
	// Delete actors marked for auto destruction
	for (TActorIterator<AActor> It(GetWorld(), AActor::StaticClass()); It; ++It)
	{
		AActor* FoundActor = *It;
		UActorComponent* AutoDestroyComponent = FoundActor->FindComponentByClass<USpatialFunctionalTestAutoDestroyComponent>();
		if (AutoDestroyComponent != nullptr)
		{
			// will be removed next frame
			FoundActor->SetLifeSpan(0.01f);
		}
	}
}

namespace
{
USpatialNetDriver* GetNetDriverAndCheckDebuggingEnabled(AActor* Actor)
{
	if (!ensureMsgf(Actor != nullptr, TEXT("Actor is null")))
	{
		return nullptr;
	}

	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(Actor->GetNetDriver());
	if (!ensureMsgf(NetDriver != nullptr, TEXT("Using SpatialFunctionalTest Debug facilities while the NetDriver is not Spatial")))
	{
		return nullptr;
	}
	if (!ensureMsgf(NetDriver->DebugCtx != nullptr,
					TEXT("SpatialFunctionalTest Debug facilities are not enabled. Enable them in your map's world settings.")))
	{
		return nullptr;
	}

	return NetDriver;
}
} // namespace

ULayeredLBStrategy* ASpatialFunctionalTest::GetLoadBalancingStrategy()
{
	UWorld* World = GetWorld();
	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(World->GetNetDriver());
	if (ensureMsgf(NetDriver != nullptr, TEXT("Trying to get a load balancing strategy while the NetDriver is not Spatial")))
	{
		if (NetDriver->DebugCtx != nullptr)
		{
			return Cast<ULayeredLBStrategy>(NetDriver->DebugCtx->DebugStrategy->GetWrappedStrategy());
		}
		else
		{
			return Cast<ULayeredLBStrategy>(NetDriver->LoadBalanceStrategy);
		}
	}
	return nullptr;
}

void ASpatialFunctionalTest::AddDebugTag(AActor* Actor, FName Tag)
{
	if (Actor == nullptr)
	{
		return;
	}

	if (USpatialNetDriver* NetDriver = GetNetDriverAndCheckDebuggingEnabled(this))
	{
		NetDriver->DebugCtx->AddActorTag(Actor, Tag);
	}
}

void ASpatialFunctionalTest::RemoveDebugTag(AActor* Actor, FName Tag)
{
	if (Actor == nullptr)
	{
		return;
	}

	if (USpatialNetDriver* NetDriver = GetNetDriverAndCheckDebuggingEnabled(this))
	{
		NetDriver->DebugCtx->RemoveActorTag(Actor, Tag);
	}
}

void ASpatialFunctionalTest::AddInterestOnTag(FName Tag)
{
	if (USpatialNetDriver* NetDriver = GetNetDriverAndCheckDebuggingEnabled(this))
	{
		NetDriver->DebugCtx->AddInterestOnTag(Tag);
	}
}

void ASpatialFunctionalTest::RemoveInterestOnTag(FName Tag)
{
	if (USpatialNetDriver* NetDriver = GetNetDriverAndCheckDebuggingEnabled(this))
	{
		NetDriver->DebugCtx->RemoveInterestOnTag(Tag);
	}
}

void ASpatialFunctionalTest::KeepActorOnCurrentWorker(AActor* Actor)
{
	if (Actor == nullptr)
	{
		return;
	}

	if (USpatialNetDriver* NetDriver = GetNetDriverAndCheckDebuggingEnabled(this))
	{
		NetDriver->DebugCtx->KeepActorOnLocalWorker(Actor);
	}
}

void ASpatialFunctionalTest::AddStepSetTagDelegation(FName Tag, int32 ServerWorkerId /*= 1*/)
{
	if (!ensureMsgf(ServerWorkerId > 0, TEXT("Invalid Server Worker Id")))
	{
		return;
	}
	if (ServerWorkerId >= GetNumExpectedServers())
	{
		ServerWorkerId = 1; // Support for single worker environments.
	}
	AddStep(FString::Printf(TEXT("Set Delegation of Tag '%s' to Server Worker %d"), *Tag.ToString(), ServerWorkerId),
			FWorkerDefinition::AllServers, nullptr, [this, Tag, ServerWorkerId] {
				SetTagDelegation(Tag, ServerWorkerId);
				FinishStep();
			});
}

void ASpatialFunctionalTest::AddStepClearTagDelegation(FName Tag)
{
	AddStep(FString::Printf(TEXT("Clear Delegation of Tag '%s'"), *Tag.ToString()), FWorkerDefinition::AllServers, nullptr, [this, Tag] {
		ClearTagDelegation(Tag);
		FinishStep();
	});
}

void ASpatialFunctionalTest::AddStepClearTagDelegationAndInterest()
{
	AddStep(TEXT("Clear Delegation of all Tags and extra Interest"), FWorkerDefinition::AllServers, nullptr, [this] {
		ClearTagDelegationAndInterest();
		FinishStep();
	});
}

void ASpatialFunctionalTest::SetTagDelegation(FName Tag, int32 ServerWorkerId)
{
	if (USpatialNetDriver* NetDriver = GetNetDriverAndCheckDebuggingEnabled(this))
	{
		NetDriver->DebugCtx->DelegateTagToWorker(Tag, ServerWorkerId);
	}
}

void ASpatialFunctionalTest::ClearTagDelegation(FName Tag)
{
	if (USpatialNetDriver* NetDriver = GetNetDriverAndCheckDebuggingEnabled(this))
	{
		NetDriver->DebugCtx->RemoveTagDelegation(Tag);
	}
}

void ASpatialFunctionalTest::ClearTagDelegationAndInterest()
{
	UWorld* World = GetWorld();
	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(World->GetNetDriver());
	if (NetDriver && NetDriver->DebugCtx)
	{
		NetDriver->DebugCtx->Reset();
	}
}

void ASpatialFunctionalTest::TakeSnapshot(const FSpatialFunctionalTestSnapshotTakenDelegate& BlueprintCallback)
{
	ISpatialGDKEditorModule* SpatialGDKEditorModule = FModuleManager::GetModulePtr<ISpatialGDKEditorModule>("SpatialGDKEditor");
	if (SpatialGDKEditorModule != nullptr)
	{
		UWorld* World = GetWorld();
		SpatialGDKEditorModule->TakeSnapshot(World, [World, BlueprintCallback](bool bSuccess, const FString& PathToSnapshot) {
			if (bSuccess)
			{
				bSuccess = SetSnapshotForMap(World, PathToSnapshot);
			}
			BlueprintCallback.ExecuteIfBound(bSuccess);
		});
	}
}

void ASpatialFunctionalTest::TakeSnapshot(const FSnapshotTakenFunc& CppCallback)
{
	ISpatialGDKEditorModule* SpatialGDKEditorModule = FModuleManager::GetModulePtr<ISpatialGDKEditorModule>("SpatialGDKEditor");
	if (SpatialGDKEditorModule != nullptr)
	{
		UWorld* World = GetWorld();
		SpatialGDKEditorModule->TakeSnapshot(World, [World, CppCallback](bool bSuccess, const FString& PathToSnapshot) {
			if (bSuccess)
			{
				bSuccess = SetSnapshotForMap(World, PathToSnapshot);
			}
			if (CppCallback != nullptr)
			{
				CppCallback(bSuccess);
			}
		});
	}
}

void ASpatialFunctionalTest::ClearSnapshot()
{
	SetSnapshotForMap(GetWorld(), FString() /* PathToSnapshot */);
}

bool ASpatialFunctionalTest::SetSnapshotForMap(UWorld* World, const FString& PathToSnapshot)
{
	check(World != nullptr);

	FString MapName = World->GetMapName();
	MapName.RemoveFromStart(World->StreamingLevelsPrefix);

	bool bSuccess = true;

	if (PathToSnapshot.IsEmpty())
	{
		TakenSnapshots.Remove(MapName);
	}
	else
	{
		FString SnapshotFileName = FString::Printf(TEXT("functional_testing_%s.snapshot"), *MapName);
		FString SnapshotSavePath = FPaths::ProjectDir() + TEXT("../spatial/snapshots/") + SnapshotFileName;
		if (FFileManagerGeneric::Get().Copy(*SnapshotSavePath, *PathToSnapshot, true, true) == 0)
		{
			TakenSnapshots.Add(MapName, SnapshotFileName);
		}
		else
		{
			bSuccess = false;
			UE_LOG(LogSpatialGDKFunctionalTests, Error, TEXT("Failed to copy snapshot file '%s' to '%s'"), *PathToSnapshot,
				   *SnapshotSavePath);
		}
	}
	return bSuccess;
}

TMap<FString, FString> ASpatialFunctionalTest::TakenSnapshots = TMap<FString, FString>();

FString ASpatialFunctionalTest::GetTakenSnapshotPath(UWorld* World)
{
	if (World == nullptr)
	{
		return FString();
	}
	FString MapName = World->GetMapName();
	MapName.RemoveFromStart(World->StreamingLevelsPrefix);
	return TakenSnapshots.FindRef(MapName);
}

bool ASpatialFunctionalTest::bWasLoadedFromTakenSnapshot = false;

void ASpatialFunctionalTest::SetLoadedFromTakenSnapshot()
{
	bWasLoadedFromTakenSnapshot = true;
}

void ASpatialFunctionalTest::ClearLoadedFromTakenSnapshot()
{
	bWasLoadedFromTakenSnapshot = false;
}

bool ASpatialFunctionalTest::WasLoadedFromTakenSnapshot()
{
	return bWasLoadedFromTakenSnapshot;
}

void ASpatialFunctionalTest::ClearAllTakenSnapshots()
{
	bWasLoadedFromTakenSnapshot = false;
	TakenSnapshots.Empty();
}
