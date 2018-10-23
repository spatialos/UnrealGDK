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

DECLARE_DELEGATE_OneParam(AcceptingPlayersDelegate, bool);

UCLASS()
class SPATIALGDK_API UGlobalStateManager : public UObject
{
	GENERATED_BODY()

public:

	void Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager);

	void ApplyData(const Worker_ComponentData& Data);
	void ApplyDeploymentMapURLData(const Worker_ComponentData& Data);
	void ApplyUpdate(const Worker_ComponentUpdate& Update);
	void ApplyDeploymentMapURLUpdate(const Worker_ComponentUpdate& Update);
	void LinkExistingSingletonActors();
	void ExecuteInitialSingletonActorReplication();
	void UpdateSingletonEntityId(const FString& ClassName, const Worker_EntityId SingletonEntityId);

	bool IsSingletonEntity(Worker_EntityId EntityId);

	void QueryGSM(bool bWithRetry);
	void SetDeploymentMapURL(const FString& MapURL);

	void SetAcceptingPlayers(bool bAcceptingPlayers);
	void AuthorityChanged(bool bWorkerAuthority, Worker_EntityId CurrentEntityID);

	FString DeploymentMapURL;
	bool bAcceptingPlayers = false;
	bool bHasLiveMapAuthority = false;

	AcceptingPlayersDelegate AcceptingPlayersChanged;

	Worker_EntityId GlobalStateManagerEntityId;

private:
	void GetSingletonActorAndChannel(FString ClassName, AActor*& OutActor, USpatialActorChannel*& OutChannel);
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
