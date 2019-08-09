// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialVirtualWorkerTranslator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialVirtualWorkerTranslator, Log, All)

class USpatialNetDriver;

typedef FString ZoneId;
typedef FString VirtualWorkerId;
typedef FString WorkerId;

UCLASS()
class USpatialVirtualWorkerTranslator : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* InNetDriver);
	/*
	void LinkExistingSingletonActor(const UClass* SingletonClass);
	void ApplyAcceptingPlayersUpdate(bool bAcceptingPlayersUpdate);
	void ApplyCanBeginPlayUpdate(const bool bCanBeginPlayUpdate);

	void BecomeAuthoritativeOverAllActors();

#if WITH_EDITOR
	void SendShutdownMultiProcessRequest();
	void SendShutdownAdditionalServersEvent();
#endif // WITH_EDITOR
*/
	void AuthorityChanged(const Worker_AuthorityChangeOp& AuthChangeOp);
	bool HandlesComponent(const Worker_ComponentId ComponentId) const;

	void ApplyWorkerComponentListenerData(const Worker_ComponentData& Data);

	void OnComponentAdded(const Worker_AddComponentOp& Op);
	void OnComponentUpdated(const Worker_ComponentUpdateOp& Op);

private:
	/*UPROPERTY()
		USpatialNetDriver* NetDriver;

	UPROPERTY()
		USpatialStaticComponentView* StaticComponentView;

	UPROPERTY()
		USpatialSender* Sender;

	UPROPERTY()
		USpatialReceiver* Receiver;

	FTimerManager* TimerManager;
	*/
	void GetZones(TArray<ZoneId>& ZoneIds);
	void GetVirtualWorkers(TArray<VirtualWorkerId>& VirtualWorkerIds);
	void GetWorkers(TArray<WorkerId>& WorkerIds);

	TMap<ZoneId, VirtualWorkerId> ZoneToVirtualWorkerMap;
	TMap<VirtualWorkerId, WorkerId> VirtualWorkerToWorkerMap;

	USpatialNetDriver* NetDriver;
};
