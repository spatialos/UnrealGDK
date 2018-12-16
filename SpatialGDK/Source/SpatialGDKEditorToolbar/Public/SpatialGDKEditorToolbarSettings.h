// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "SpatialGDKEditorSettings.h"

#include "Engine/EngineTypes.h"

#include "SpatialGDKEditorToolbarSettings.generated.h"

USTRUCT()
struct FInteropTypebindingInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false))
	UClass* ReplicatedClass;

	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false))
	TArray<FString> IncludeList;
};

UCLASS(config = EditorPerProjectUserSettings, defaultconfig)
class USpatialGDKEditorToolbarSettings : public USpatialGDKEditorSettings
{
	GENERATED_BODY()

public:
	USpatialGDKEditorToolbarSettings(const FObjectInitializer& ObjectInitializer);

public:
	/** Launch configuration file used for `spatial local launch`. */
	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false, DisplayName = "Launch configuration"))
	FString SpatialOSLaunchConfig;

	/** Stop `spatial local launch` when shutting down editor. */
	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false, DisplayName = "Stop on exit"))
	bool bStopSpatialOnExit;

private:
	/** Name of your SpatialOS snapshot file. */
	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false, DisplayName = "Snapshot file name"))
	FString SpatialOSSnapshotFile;

public:
	UFUNCTION()
	FORCEINLINE FString GetSpatialOSSnapshotFile() const
	{
		return SpatialOSSnapshotFile.IsEmpty()
			? FString(TEXT("default.snapshot"))
			: SpatialOSSnapshotFile;
	}

	FString ToString() override;
};

