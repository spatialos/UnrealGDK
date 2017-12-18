// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "IpNetDriver.h"
#include "SpatialUpdateInterop.h"
#include "SpatialNetDriver.generated.h"

class UEntityPipeline;
class UEntityRegistry;
class UCallbackDispatcher;
class USpatialOSComponentUpdater;
class USpatialOS;

UCLASS()
class NUF_API USpatialNetDriver : public UIpNetDriver
{
	GENERATED_BODY()

public:
	virtual bool InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error) override;
	virtual int32 ServerReplicateActors(float DeltaSeconds) override;
	virtual void TickDispatch(float DeltaTime) override;
	virtual void ProcessRemoteFunction(class AActor* Actor, class UFunction* Function, void* Parameters, struct FOutParmRec* OutParms, struct FFrame* NotStack, class UObject* SubObject = NULL ) override;

	USpatialOS* GetSpatialOS() const
	{
		return SpatialOSInstance;
	}

private:	
	UPROPERTY()
	USpatialOS* SpatialOSInstance;

	UPROPERTY()
	USpatialOSComponentUpdater* SpatialOSComponentUpdater;

	UPROPERTY()
	UEntityRegistry* EntityRegistry;

	// Update interop.
	UPROPERTY()
	USpatialUpdateInterop* UpdateInterop;

	UFUNCTION()
	void OnSpatialOSConnected();

	UFUNCTION()
	void OnSpatialOSConnectFailed();

	UFUNCTION()
	void OnSpatialOSDisconnected();

	void ProcessServerMove(struct FFrame* TempRpcFrameForReading, worker::EntityId entityId, UPackageMap* PackageMap);
	void ProcessClientAckGoodMove(struct FFrame* TempRpcFrameForReading, worker::EntityId entityId);
};
