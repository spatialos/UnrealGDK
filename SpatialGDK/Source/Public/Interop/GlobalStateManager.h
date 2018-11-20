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

	void ApplyData(const Worker_ComponentData& Data);
	void ApplyDeploymentMapURLData(const Worker_ComponentData& Data);
	void ApplyUpdate(const Worker_ComponentUpdate& Update);
	void ApplyDeploymentMapUpdate(const Worker_ComponentUpdate& Update);
	void LinkExistingSingletonActors();
	void ExecuteInitialSingletonActorReplication();
	void UpdateSingletonEntityId(const FString& ClassName, const Worker_EntityId SingletonEntityId);

	bool IsSingletonEntity(Worker_EntityId EntityId);

	void QueryGSM(bool bRetryUntilAcceptingPlayers);
	void RetryQueryGSM(bool bRetryUntilAcceptingPlayers);
	bool GetAcceptingPlayersFromQueryResponse(Worker_EntityQueryResponseOp& Op);
	void ApplyDeploymentMapDataFromQueryResponse(Worker_EntityQueryResponseOp& Op);
	void SetDeploymentMapURL(const FString& MapURL);

	void SetAcceptingPlayers(bool bAcceptingPlayers);
	void AuthorityChanged(bool bWorkerAuthority, Worker_EntityId CurrentEntityID);

	USpatialActorChannel* AddSingleton(AActor* SingletonActor);

	FString DeploymentMapURL;
	bool bAcceptingPlayers = false;

	Worker_EntityId GlobalStateManagerEntityId;

private:
	void LinkExistingSingletonActor(const UClass* SingletonClass);
	void ApplyAcceptingPlayersUpdate(bool bAcceptingPlayersUpdate);

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	USpatialStaticComponentView* StaticComponentView;

	UPROPERTY()
	USpatialSender* Sender;

	UPROPERTY()
	USpatialReceiver* Receiver;

	StringToEntityMap SingletonNameToEntityId;

	FTimerManager* TimerManager;
};
