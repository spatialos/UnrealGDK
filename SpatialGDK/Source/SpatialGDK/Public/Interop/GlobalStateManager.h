// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
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
	void Init(USpatialNetDriver* InNetDriver);

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

	DECLARE_DELEGATE_OneParam(QueryDelegate, const Worker_EntityQueryResponseOp&);
	void QueryGSM(const QueryDelegate& Callback);
	bool GetAcceptingPlayersAndSessionIdFromQueryResponse(const Worker_EntityQueryResponseOp& Op, bool& OutAcceptingPlayers, int32& OutSessionId);
	void ApplyVirtualWorkerMappingFromQueryResponse(const Worker_EntityQueryResponseOp& Op);
	void ApplyDeploymentMapDataFromQueryResponse(const Worker_EntityQueryResponseOp& Op);

	void SetDeploymentState();
	void SetAcceptingPlayers(bool bAcceptingPlayers);
	void SetCanBeginPlay(const bool bInCanBeginPlay);
	void IncrementSessionID();

	FORCEINLINE FString GetDeploymentMapURL() const { return DeploymentMapURL; }
	FORCEINLINE bool GetAcceptingPlayers() const { return bAcceptingPlayers; }
	FORCEINLINE int32 GetSessionId() const { return DeploymentSessionId; }
	FORCEINLINE uint32 GetSchemaHash() const { return SchemaHash; }

	void AuthorityChanged(const Worker_AuthorityChangeOp& AuthChangeOp);
	bool HandlesComponent(const Worker_ComponentId ComponentId) const;

	void ResetGSM();

	void BeginDestroy() override;

	bool HasAuthority();

	void TriggerBeginPlay();

	FORCEINLINE bool GetCanBeginPlay() const
	{
		return bCanBeginPlay;
	}

	USpatialActorChannel* AddSingleton(AActor* SingletonActor);
	void RegisterSingletonChannel(AActor* SingletonActor, USpatialActorChannel* SingletonChannel);
	void RemoveSingletonInstance(const AActor* SingletonActor);

	Worker_EntityId GlobalStateManagerEntityId;

	// Singleton Manager Component
	StringToEntityMap SingletonNameToEntityId;

private:
	// Deployment Map Component
	FString DeploymentMapURL;
	bool bAcceptingPlayers;
	int32 DeploymentSessionId = 0;
	uint32 SchemaHash;

	// Startup Actor Manager Component
	bool bCanBeginPlay;
	bool bCanSpawnWithAuthority;

public:
#if WITH_EDITOR
	void OnPrePIEEnded(bool bValue);
	void ReceiveShutdownMultiProcessRequest();

	void OnShutdownComponentUpdate(const Worker_ComponentUpdate& Update);
	void ReceiveShutdownAdditionalServersEvent();
#endif // WITH_EDITOR
private:
	void SetDeploymentMapURL(const FString& MapURL);
	void SendSessionIdUpdate();
	void LinkExistingSingletonActor(const UClass* SingletonClass);
	void ApplyCanBeginPlayUpdate(const bool bCanBeginPlayUpdate);

	void BecomeAuthoritativeOverAllActors();
	void BecomeAuthoritativeOverActorsBasedOnLBStrategy();

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

	FDelegateHandle PrePIEEndedHandle;

	TMap<FString, TPair<AActor*, USpatialActorChannel*>> SingletonClassPathToActorChannels;
};
