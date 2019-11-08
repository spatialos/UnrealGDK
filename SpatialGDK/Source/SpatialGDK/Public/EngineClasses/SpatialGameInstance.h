// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "SpatialGameInstance.generated.h"

class USpatialWorkerConnection;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGameInstance, Log, All);

DECLARE_EVENT(USpatialWorkerConnection, FOnConnectedEvent);
DECLARE_EVENT_OneParam(USpatialWorkerConnection, FOnConnectionFailedEvent, const FString&);

UCLASS(config = Engine)
class SPATIALGDK_API USpatialGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual FGameInstancePIEResult StartPlayInEditorGameInstance(ULocalPlayer* LocalPlayer, const FGameInstancePIEParameters& Params) override;
#endif
	virtual void StartGameInstance() override;

	//~ Begin UObject Interface
	virtual bool ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor) override;
	//~ End UObject Interface

	// bResponsibleForSnapshotLoading exists to have persistent knowledge if this worker has authority over the GSM during ServerTravel.
	bool bResponsibleForSnapshotLoading = false;

	// The SpatialWorkerConnection must always be owned by the SpatialGameInstance and so must be created here to prevent TrimMemory from deleting it during Browse.
	void CreateNewSpatialWorkerConnection();

	// Destroying the SpatialWorkerConnection disconnects us from SpatialOS.
	void DestroySpatialWorkerConnection();

	FORCEINLINE USpatialWorkerConnection* GetSpatialWorkerConnection() { return SpatialConnection; }

	void HandleOnConnected();
	void HandleOnConnectionFailed(const FString& Reason);

	// Invoked when this worker has successfully connected to SpatialOS
	FOnConnectedEvent OnConnected;
	// Invoked when this worker fails to initiate a connection to SpatialOS
	FOnConnectionFailedEvent OnConnectionFailed;

	void SetFirstConnectionToSpatialOSAttempted() { bFirstConnectionToSpatialOSAttempted = true; };
	bool GetFirstConnectionToSpatialOSAttempted() const { return bFirstConnectionToSpatialOSAttempted; };

protected:
	// Checks whether the current net driver is a USpatialNetDriver.
	// Can be used to decide whether to use Unreal networking or SpatialOS networking.
	bool HasSpatialNetDriver() const;

private:
	// SpatialConnection is stored here for persistence between map travels.
	UPROPERTY()
	USpatialWorkerConnection* SpatialConnection;

	bool bFirstConnectionToSpatialOSAttempted = false;

	// If this flag is set to true standalone clients will not attempt to connect to a deployment automatically if a 'loginToken' exists in arguments.
	UPROPERTY(Config)
	bool bPreventAutoConnectWithLocator;
};
