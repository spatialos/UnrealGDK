// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "UObject/NoExportTypes.h"

#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "GlobalStateManager.generated.h"

class USpatialNetDriver;
class USpatialActorChannel;
class USpatialStaticComponentView;
class USpatialSender;
class USpatialReceiver;

DECLARE_LOG_CATEGORY_EXTERN(LogGlobalStateManager, Log, All)

UCLASS()
class SPATIALGDK_API UGlobalStateManager : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager);

	void ApplySingletonManagerData(const Worker_ComponentData& Data);
	void ApplyDeploymentMapData(const Worker_ComponentData& Data);
	void ApplyStartupActorManagerData(const Worker_ComponentData& Data);

	void ApplySingletonManagerUpdate(const Worker_ComponentUpdate& Update);
	void ApplyDeploymentMapUpdate(const Worker_ComponentUpdate& Update);
	void ApplyStartupActorManagerUpdate(const Worker_ComponentUpdate& Update);

	bool IsSingletonEntity(Worker_EntityId EntityId) const;
	void LinkAllExistingSingletonActors();
	void ExecuteInitialSingletonActorReplication();
	void UpdateSingletonEntityId(const FString& ClassName, const Worker_EntityId SingletonEntityId);

	void QueryGSM(bool bRetryUntilAcceptingPlayers);
	void RetryQueryGSM(bool bRetryUntilAcceptingPlayers);
	bool GetAcceptingPlayersFromQueryResponse(Worker_EntityQueryResponseOp& Op);
	void ApplyDeploymentMapDataFromQueryResponse(Worker_EntityQueryResponseOp& Op);
	void SetDeploymentMapURL(const FString& MapURL);

	void SetAcceptingPlayers(bool bAcceptingPlayers);
	void SetCanBeginPlay(bool bInCanBeginPlay);

	void AuthorityChanged(bool bWorkerAuthority, Worker_EntityId CurrentEntityID);

	void BeginDestroy() override;

	bool HasAuthority();

	USpatialActorChannel* AddSingleton(AActor* SingletonActor);

	Worker_EntityId GlobalStateManagerEntityId;

	// Singleton Manager Component
	StringToEntityMap SingletonNameToEntityId;

	// Deployment Map Component
	FString DeploymentMapURL;
	bool bAcceptingPlayers;

	// Startup Actor Manager Component
	bool bCanBeginPlay;

#if WITH_EDITOR
	void OnPrePIEEnded(bool bValue);
	void ReceiveShutdownMultiProcessRequest();

	void OnShutdownComponentUpdate(Worker_ComponentUpdate& Update);
	void ReceiveShutdownAdditionalServersEvent();
#endif // WITH_EDITOR
private:
	void LinkExistingSingletonActor(const UClass* SingletonClass);
	void ApplyAcceptingPlayersUpdate(bool bAcceptingPlayersUpdate);
	void ApplyCanBeginPlayUpdate(bool bCanBeginPlayUpdate);

	void BecomeAuthoritativeOverAllActors();
	void TriggerBeginPlay();

#if WITH_EDITOR
	void SendShutdownMultiProcessRequest();
	void SendShutdownAdditionalServersEvent();
#endif // WITH_EDITOR

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	USpatialStaticComponentView* StaticComponentView;

	UPROPERTY()
	USpatialSender* Sender;

	UPROPERTY()
	USpatialReceiver* Receiver;

	FTimerManager* TimerManager;

	bool bTriggeredBeginPlay;
};
