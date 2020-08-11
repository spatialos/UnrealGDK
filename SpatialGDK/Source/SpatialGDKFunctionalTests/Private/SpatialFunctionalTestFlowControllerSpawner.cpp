#include "SpatialFunctionalTestFlowControllerSpawner.h"

#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "GameFramework/PlayerController.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "SpatialFunctionalTest.h"
#include "SpatialFunctionalTestFlowController.h"

SpatialFunctionalTestFlowControllerSpawner::SpatialFunctionalTestFlowControllerSpawner()
	: SpatialFunctionalTestFlowControllerSpawner(
		nullptr, TSubclassOf<ASpatialFunctionalTestFlowController>(ASpatialFunctionalTestFlowController::StaticClass()))
{
}

SpatialFunctionalTestFlowControllerSpawner::SpatialFunctionalTestFlowControllerSpawner(
	ASpatialFunctionalTest* ControllerOwningTest, TSubclassOf<ASpatialFunctionalTestFlowController> FlowControllerClassToSpawn)
	: OwningTest(ControllerOwningTest)
	, FlowControllerClass(FlowControllerClassToSpawn)
	, NextClientControllerId(1)
{
}

void SpatialFunctionalTestFlowControllerSpawner::ModifyFlowControllerClassToSpawn(
	TSubclassOf<ASpatialFunctionalTestFlowController> FlowControllerClassToSpawn)
{
	FlowControllerClass = FlowControllerClassToSpawn;
}

ASpatialFunctionalTestFlowController* SpatialFunctionalTestFlowControllerSpawner::SpawnServerFlowController()
{
	UWorld* World = OwningTest->GetWorld();
	UNetDriver* NetDriver = World->GetNetDriver();
	if (NetDriver != nullptr && !NetDriver->IsServer())
	{
		// Not a server, quit
		return nullptr;
	}

	ASpatialFunctionalTestFlowController* ServerFlowController =
		World->SpawnActorDeferred<ASpatialFunctionalTestFlowController>(FlowControllerClass, FTransform());
	ServerFlowController->OwningTest = OwningTest;
	ServerFlowController->WorkerDefinition = FWorkerDefinition{ ESpatialFunctionalTestWorkerType::Server, OwningServerIntanceId(World) };

	ServerFlowController->FinishSpawning(FTransform(), true);
	// TODO: Replace locking with custom LB strategy - UNR-3393
	LockFlowControllerDelegations(ServerFlowController);

	return ServerFlowController;
}

ASpatialFunctionalTestFlowController* SpatialFunctionalTestFlowControllerSpawner::SpawnClientFlowController(APlayerController* OwningClient)
{
	UWorld* World = OwningTest->GetWorld();

	ASpatialFunctionalTestFlowController* FlowController =
		World->SpawnActorDeferred<ASpatialFunctionalTestFlowController>(FlowControllerClass, OwningTest->GetActorTransform(), OwningClient);
	FlowController->OwningTest = OwningTest;
	FlowController->WorkerDefinition =
		FWorkerDefinition{ ESpatialFunctionalTestWorkerType::Client,
						   INVALID_FLOW_CONTROLLER_ID }; // by default have invalid id, Test Authority will set it to ensure uniqueness

	FlowController->FinishSpawning(OwningTest->GetActorTransform(), true);
	// TODO: Replace locking with custom LB strategy - UNR-3393
	LockFlowControllerDelegations(FlowController);

	return FlowController;
}

void SpatialFunctionalTestFlowControllerSpawner::AssignClientFlowControllerId(ASpatialFunctionalTestFlowController* ClientFlowController)
{
	check(OwningTest->HasAuthority() && ClientFlowController != nullptr
		  && ClientFlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Client
		  && ClientFlowController->WorkerDefinition.Id == INVALID_FLOW_CONTROLLER_ID);

	ClientFlowController->CrossServerSetWorkerId(NextClientControllerId++);
}

uint8 SpatialFunctionalTestFlowControllerSpawner::OwningServerIntanceId(UWorld* World) const
{
	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(World->GetNetDriver());
	if (SpatialNetDriver == nullptr || SpatialNetDriver->LoadBalanceStrategy == nullptr)
	{
		// not load balanced test, default instance 1
		return 1;
	}
	else
	{
		return static_cast<uint8>(SpatialNetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId());
	}
}

void SpatialFunctionalTestFlowControllerSpawner::LockFlowControllerDelegations(ASpatialFunctionalTestFlowController* FlowController) const
{
	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(FlowController->GetNetDriver());
	if (SpatialNetDriver == nullptr || SpatialNetDriver->LoadBalanceStrategy == nullptr)
	{
		return;
	}
	else
	{
		SpatialNetDriver->LockingPolicy->AcquireLock(FlowController);
	}
}
