// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "SpatialGameInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGameInstance, Log, All);

class USpatialWorkerConnection;

UCLASS()
class SPATIALGDK_API USpatialGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual FGameInstancePIEResult StartPlayInEditorGameInstance(ULocalPlayer* LocalPlayer, const FGameInstancePIEParameters& Params) override;
#endif
	virtual void StartGameInstance() override;
	virtual void Shutdown() override;
	void QuitGame();

	// bResponsibleForSnapshotLoading exists to have persistent knowledge if this worker has authority over the GSM during ServerTravel.
	bool bResponsibleForSnapshotLoading = false;

	// The SpatialWorkerConnection must always be owned by the SpatialGameInstance and so must be created here to prevent TrimMemory from deleting it during Browse.
	void CreateNewSpatialWorkerConnection();

protected:
	// Checks whether the current net driver is a USpatialNetDriver.
	// Can be used to decide whether to use Unreal networking or SpatialOS networking.
	bool HasSpatialNetDriver() const;
	// Helper function that bypasses some of the Unreal flow (which won't work with the SpatialOS model) when launching a new game as a client.
	bool StartGameInstance_SpatialGDKClient(FString& Error);

private:

	// TODO: Move SpatialConnection ownership to NetDriver
	friend class USpatialNetDriver;
	UPROPERTY()
	USpatialWorkerConnection* SpatialConnection;
};
