// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "IpNetDriver.h"
#include "SpatialInteropBlock.h"
#include "SpatialUpdateInterop.h"
#include "PlayerSpawnRequestSender.h"
#include "SpatialNetDriver.generated.h"

class UEntityPipeline;
class UEntityRegistry;
class UCallbackDispatcher;
class USpatialOSComponentUpdater;
class USpatialOS;
class USpatialNetConnection;

// SpatialNetDriver will not be in the NUF module in a final product, so we can merge this with LogSpatialOS.
DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOSNUF, Log, All);

UCLASS()
class NUF_API USpatialNetDriver : public UIpNetDriver
{
	GENERATED_BODY()

public:
	virtual void PostInitProperties() override;

	// Begin UNetDriver interface.
	virtual bool InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error) override;
	virtual int32 ServerReplicateActors(float DeltaSeconds) override;
	virtual void TickDispatch(float DeltaTime) override;
	virtual void ProcessRemoteFunction(class AActor* Actor, class UFunction* Function, void* Parameters, struct FOutParmRec* OutParms, struct FFrame* NotStack, class UObject* SubObject = NULL ) override;
	virtual void TickFlush(float DeltaTime) override;
	virtual bool IsLevelInitializedForActor(const AActor* InActor, const UNetConnection* InConnection) const override;
	// End UNetDriver interface.

	// Set the timer manager.
	void SetTimerManager(FTimerManager* InTimerManager);

	FTimerManager& GetTimerManager() const
	{
		return *TimerManager;
	}

	USpatialOS* GetSpatialOS() const
	{
		return SpatialOSInstance;
	}

	// Returns the "100% reliable" connection to SpatialOS.
	// On the server, it is designated to be the first client connection.
	// On the client, this function is not meaningful (as we use ServerConnection)
	USpatialNetConnection* GetSpatialOSNetConnection() const;

	UPROPERTY()
	USpatialInteropBlock* SpatialInteropBlock;

	UEntityRegistry* GetEntityRegistry() { return EntityRegistry; }

	USpatialOS* GetSpatialOS() { return SpatialOSInstance; }
	
	bool AcceptNewPlayer(const FURL& InUrl);

	USpatialUpdateInterop* GetSpatialUpdateInterop() const
	{
		return UpdateInterop;
	}

protected:
	UPROPERTY()
	USpatialOS* SpatialOSInstance;

	UPROPERTY()
	USpatialOSComponentUpdater* SpatialOSComponentUpdater;

	UPROPERTY()
	UEntityRegistry* EntityRegistry;

	// Timer manager.
	FTimerManager* TimerManager;

	// Update interop.
	UPROPERTY()
	USpatialUpdateInterop* UpdateInterop;

	UFUNCTION()
	void OnSpatialOSConnected();

	UFUNCTION()
	void OnSpatialOSConnectFailed();

	UFUNCTION()
	void OnSpatialOSDisconnected();
		
#if WITH_SERVER_CODE
	//NUF: These functions all exist in UNetDriver, but we need to modify/simplify them in certain ways.
	// Could have marked them virtual in base class but that's a pointless source change as these functions are not meant to be called from anywhere except USpatialNetDriver::ServerReplicateActors.
	int32 ServerReplicateActors_PrepConnections(const float DeltaSeconds);
	int32 ServerReplicateActors_PrioritizeActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, const TArray<FNetworkObjectInfo*> ConsiderList, const bool bCPUSaturated, FActorPriority*& OutPriorityList, FActorPriority**& OutPriorityActors);
	int32 ServerReplicateActors_ProcessPrioritizedActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, FActorPriority** PriorityActors, const int32 FinalSortedCount, int32& OutUpdated);
#endif

private:
	FPlayerSpawnRequestSender PlayerSpawner;
};
