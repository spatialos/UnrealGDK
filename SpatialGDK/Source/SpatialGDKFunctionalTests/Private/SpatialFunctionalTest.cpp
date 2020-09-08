// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialFunctionalTest.h"

#include "../../SpatialGDKServices/Public/LocalDeploymentManager.h"
#include "../../SpatialGDKServices/Public/SpatialGDKServicesModule.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialNetDriverDebugContext.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManagerGeneric.h"
#include "HttpModule.h"
#include "Interfaces/IHttpBase.h"
#include "Interfaces/IHttpResponse.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "LoadBalancing/DebugLBStrategy.h"
#include "LoadBalancing/LayeredLBStrategy.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTestAutoDestroyComponent.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTestsPrivate.h"
#include "TimerManager.h"

#pragma optimize("", off)

ASpatialFunctionalTest::ASpatialFunctionalTest()
	: Super()
	, FlowControllerSpawner(this, ASpatialFunctionalTestFlowController::StaticClass())
{
	bReplicates = true;
	NetPriority = 3.0f;
	NetUpdateFrequency = 100.0f;

	bAlwaysRelevant = true;

	PrimaryActorTick.TickInterval = 0.0f;
}

void ASpatialFunctionalTest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
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
	auto* AuxLocalFlowController = GetLocalFlowController();
	ensureMsgf(AuxLocalFlowController != nullptr, TEXT("Can't Find LocalFlowController"));
	if (AuxLocalFlowController != nullptr)
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

			Super::FinishTest(TestResult, Message);
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

// Add Steps for Blueprints

void ASpatialFunctionalTest::AddStepBlueprint(const FString& StepName, const FWorkerDefinition& Worker,
											  const FStepIsReadyDelegate& IsReadyEvent, const FStepStartDelegate& StartEvent,
											  const FStepTickDelegate& TickEvent, float StepTimeLimit /*= 0.0f*/)
{
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
	FSpatialFunctionalTestStepDefinition StepDefinitionCopy = StepDefinition;

	StepDefinitionCopy.Workers.Add(Worker);

	StepDefinitions.Add(StepDefinitionCopy);
}

void ASpatialFunctionalTest::AddStepFromDefinitionMulti(const FSpatialFunctionalTestStepDefinition& StepDefinition,
														const TArray<FWorkerDefinition>& Workers)
{
	FSpatialFunctionalTestStepDefinition StepDefinitionCopy = StepDefinition;

	StepDefinitionCopy.Workers.Append(Workers);

	StepDefinitions.Add(StepDefinitionCopy);
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

void ASpatialFunctionalTest::CrossServerNotifyStepFinished_Implementation(ASpatialFunctionalTestFlowController* FlowController)
{
	if (CurrentStepIndex < 0)
	{
		return;
	}

	const FString FLowControllerDisplayName = FlowController->GetDisplayName();

	UE_LOG(LogSpatialGDKFunctionalTests, Display, TEXT("%s finished Step"), *FLowControllerDisplayName);

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
		// test finished
		if (StartTime > 0)
		{
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
		}
		if (!HasAuthority()) // Authority already does this on Super::FinishTest
		{
			NotifyTestFinishedObserver();
		}

		DeleteActorsRegisteredForAutoDestroy();
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
	GetWorld()->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateLambda([this](AActor* Spawned) {
		if (APlayerController* PlayerController = Cast<APlayerController>(Spawned))
		{
			if (PlayerController->HasAuthority())
			{
				this->FlowControllerSpawner.SpawnClientFlowController(PlayerController);
			}
		}
	}));
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

#pragma optimize("", on)
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

void ASpatialFunctionalTest::DelegateTagToWorker(FName Tag, int32 WorkerId)
{
	if (USpatialNetDriver* NetDriver = GetNetDriverAndCheckDebuggingEnabled(this))
	{
		NetDriver->DebugCtx->DelegateTagToWorker(Tag, WorkerId);
	}
}

void ASpatialFunctionalTest::RemoveTagDelegation(FName Tag)
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

void ASpatialFunctionalTest::TakeSnapshot(const FSnapshotTakenDelegate& BlueprintCallback)
{
	TakeSnapshot(BlueprintCallback, nullptr);
}

void ASpatialFunctionalTest::TakeSnapshot(const FSnapshotTakenFunc& CppCallback)
{
	TakeSnapshot(FSnapshotTakenDelegate(), CppCallback);
}

void ASpatialFunctionalTest::TakeSnapshot(const FSnapshotTakenDelegate& BlueprintCallback, const FSnapshotTakenFunc& CppCallback)
{
	FHttpModule& HttpModule = FModuleManager::LoadModuleChecked<FHttpModule>("HTTP");
	TSharedRef<class IHttpRequest> HttpRequest = HttpModule.Get().CreateRequest();
	// FString KrakenSnapshotURL = "http://localhost:31000/improbable.platform.runtime.SnapshotService/TakeSnapshot";
	// HttpRequest->OnProcessRequestComplete().BindLambda([this, BlueprintCallback, CppCallback](
	//													   FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded) {
	//	if (!bSucceeded)
	//	{
	//		UE_LOG(LogSpatialGDKFunctionalTests, Error, TEXT("Failed to trigger snapshot at '%s'; received '%s'"), *HttpRequest->GetURL(),
	//			   *HttpResponse->GetContentAsString());
	//		BlueprintCallback.ExecuteIfBound(false);
	//		if (CppCallback != nullptr)
	//		{
	//			CppCallback(false);
	//		}
	//		return;
	//	}

	//	// Unfortunately by the time this callback happens, the files haven't been flushed, so if you copy you may get
	//	// the wrong info! So let's wait a bit..

	//	FTimerHandle TimerHandle;
	//	GetWorld()->GetTimerManager().SetTimer(
	//		TimerHandle,
	//		[BlueprintCallback, CppCallback]() {
	//			bool bSuccess = false;

	//			// Go read latest file,
	//			FString AppDataLocalPath = FPlatformMisc::GetEnvironmentVariable(TEXT("LOCALAPPDATA"));
	//			FString LatestSnapshotInfoPath = FString::Printf(TEXT("%s/.improbable/local_snapshots/latest"), *AppDataLocalPath);
	//			FString LatestSnapshot;
	//			if (FPaths::FileExists(LatestSnapshotInfoPath) && FFileHelper::LoadFileToString(LatestSnapshot, *LatestSnapshotInfoPath))
	//			{
	//				FString LatestSnapshotPath =
	//					FString::Printf(TEXT("%s/.improbable/local_snapshots/%s"), *AppDataLocalPath, *LatestSnapshot);

	//				// Currently there's a limitation that snapshots can only be read from this folder and you
	//				// can only pass file name.
	//				FString SnapshotSavePath = FPaths::ProjectDir() + "../spatial/snapshots/functional_testing.snapshot";

	//				if (FFileManagerGeneric::Get().Copy(*SnapshotSavePath, *LatestSnapshotPath, true, true) == 0)
	//				{
	//					bSuccess = true;
	//					ASpatialFunctionalTest::TakenSnapshotPath = TEXT("functional_testing.snapshot");
	//				}
	//				else
	//				{
	//					UE_LOG(LogSpatialGDKFunctionalTests, Error, TEXT("Failed to copy snapshot file '%s' to '%s'"),
	//						   *LatestSnapshotInfoPath, *SnapshotSavePath);
	//				}
	//			}
	//			else
	//			{
	//				UE_LOG(LogSpatialGDKFunctionalTests, Error,
	//					   TEXT("Couldn't find or read the file with info of which is the latest snapshot '%s'"), *LatestSnapshotInfoPath);
	//			}

	//			BlueprintCallback.ExecuteIfBound(bSuccess);
	//			if (CppCallback != nullptr)
	//			{
	//				CppCallback(bSuccess);
	//			}
	//		},
	//		0.1f, false);
	//});
	// HttpRequest->SetURL(KrakenSnapshotURL);
	// HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/grpc-web+proto"));
	// HttpRequest->SetVerb(TEXT("POST"));
	// const TArray<uint8> Body = { 0, 0, 0, 0, 0 };
	// HttpRequest->SetContent(Body);
	// HttpRequest->ProcessRequest();
	FString SquidSnapshotUrl = TEXT("http://localhost:5006/snapshot");
	HttpRequest->OnProcessRequestComplete().BindLambda([this, BlueprintCallback, CppCallback](
														   FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded) {
		if (!bSucceeded)
		{
			UE_LOG(LogSpatialGDKFunctionalTests, Error, TEXT("Failed to trigger snapshot at '%s'; received '%s'"), *HttpRequest->GetURL(),
				   *HttpResponse->GetContentAsString());
			BlueprintCallback.ExecuteIfBound(false);
			if (CppCallback != nullptr)
			{
				CppCallback(false);
			}
			return;
		}

		// Unfortunately by the time this callback happens, the files haven't been flushed, so if you copy you may get
		// the wrong info! So let's wait a bit..

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[BlueprintCallback, CppCallback]() {
				bool bSuccess = false;

				FSpatialGDKServicesModule& GDKServices = FModuleManager::GetModuleChecked<FSpatialGDKServicesModule>("SpatialGDKServices");
				FLocalDeploymentManager* LocalDeploymentManager = GDKServices.GetLocalDeploymentManager();

				FString SpatialPath = FPaths::ProjectDir() + TEXT("../spatial/");
				FString CurrentLocalDeploymentPath =
					SpatialPath + TEXT("logs/localdeployment/") + LocalDeploymentManager->GetLocalRunningDeploymentID();

				IFileManager& FileManager = FFileManagerGeneric::Get();

				TArray<FString> SnapshotFiles;

				FileManager.FindFiles(SnapshotFiles, *CurrentLocalDeploymentPath, TEXT("snapshot"));

				FString NewestSnapshotFilePath;

				FDateTime NewestSnapshotTimestamp = FDateTime::MinValue();

				for (const FString& SnapshotFile : SnapshotFiles)
				{
					FString SnapshotFilePath = CurrentLocalDeploymentPath + TEXT("/") + SnapshotFile;
					FDateTime SnapshotFileTimestamp = FileManager.GetTimeStamp(*SnapshotFilePath);
					if (SnapshotFileTimestamp > NewestSnapshotTimestamp)
					{
						NewestSnapshotTimestamp = SnapshotFileTimestamp;
						NewestSnapshotFilePath = SnapshotFilePath;
					}
				}

				bSuccess = !NewestSnapshotFilePath.IsEmpty();

				if (bSuccess)
				{
					FString SnapshotFileName = TEXT("functional_testing.snapshot");
					FString SnapshotSavePath = FPaths::ProjectDir() + TEXT("../spatial/snapshots/") + SnapshotFileName;
					if (FFileManagerGeneric::Get().Copy(*SnapshotSavePath, *NewestSnapshotFilePath, true, true) == 0)
					{
						bSuccess = true;
						ASpatialFunctionalTest::TakenSnapshotPath = SnapshotFileName;
					}
					else
					{
						UE_LOG(LogSpatialGDKFunctionalTests, Error, TEXT("Failed to copy snapshot file '%s' to '%s'"),
							   *NewestSnapshotFilePath, *SnapshotSavePath);
					}
				}
				else
				{
					UE_LOG(LogSpatialGDKFunctionalTests, Error, TEXT("Failed to find newest snapshot in '%s'"),
						   *CurrentLocalDeploymentPath);
				}

				BlueprintCallback.ExecuteIfBound(bSuccess);
				if (CppCallback != nullptr)
				{
					CppCallback(bSuccess);
				}
			},
			0.5f, false);
	});
	HttpRequest->SetURL(SquidSnapshotUrl);
	HttpRequest->SetVerb("GET");
	HttpRequest->SetHeader("Content-Type", TEXT("application/json"));
	HttpRequest->ProcessRequest();
}

FString ASpatialFunctionalTest::TakenSnapshotPath = "";

FString ASpatialFunctionalTest::GetTakenSnapshotPath()
{
	return TakenSnapshotPath;
}

bool ASpatialFunctionalTest::WasLoadedFromSnapshot()
{
	return bWasLoadedFromSnapshot;
}

void ASpatialFunctionalTest::ClearLoadedFromSnapshot()
{
	bWasLoadedFromSnapshot = false;
	TakenSnapshotPath = "";
}

void ASpatialFunctionalTest::SetLoadedFromSnapshot()
{
	bWasLoadedFromSnapshot = true;

	checkf(!TakenSnapshotPath.IsEmpty(), TEXT("SetLoadedFromSnapshot but there's not snapshot path"));
}

bool ASpatialFunctionalTest::bWasLoadedFromSnapshot = false;
