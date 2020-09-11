// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/SpatialMultiWorkerSettings.h"

#include "GameFramework/WorldSettings.h"
#include "Templates/SubclassOf.h"

#include "SpatialWorldSettings.generated.h"

/**
 * MapTestingMode allows Maps using ASpatialWorldSettings to define how Tests should run by the Automation Manager.
 * It will handle it in a way that the current Project Settings will remain untouched.
 */
UENUM()
enum class EMapTestingMode : uint8
{
	// It will search for ASpatialFunctionalTest, if there are any it forces Spatial otherwise Native
	Detect = 0,
	// Forces Spatial to be off and Play Offline (1 Client, no network), the default Native behaviour
	ForceNativeOffline = 1,
	// Forces Spatial to be off and Play As Listen Server (1 Client that is also Server, ie authoritive Client)
	ForceNativeAsListenServer = 2,
	// Forces Spatial to be off and Play As Client (1 Client + 1 Dedicated Server)
	ForceNativeAsClient = 3,
	// Forces Spatial to be on. Calculates the number of players needed based on the ASpatialFunctionalTests present, 1 if none exists
	ForceSpatial = 4,
	// Uses current settings to run the tests
	UseCurrentSettings = 5
};

USTRUCT(BlueprintType)
struct FMapTestingSettings
{
	GENERATED_BODY();

	/* Available Modes to run Tests:
	- Detect: It will search for ASpatialFunctionalTest, if there are any it forces Spatial otherwise Native
	- Force Native: Forces Spatial to be off and use only 1 Client, the default Native behaviour
	- Force Spatial: Forces Spatial to be on. Calculates the number of players needed based on the ASpatialFunctionalTests present, 1 if
	none exists
	- Use Current Settings: Uses current settings to run the tests */
	UPROPERTY(EditAnywhere, Category = "Default")
	EMapTestingMode TestingMode = EMapTestingMode::Detect;
};

UCLASS()
class SPATIALGDK_API ASpatialWorldSettings : public AWorldSettings
{
	GENERATED_UCLASS_BODY()
	friend class USpatialStatics;

public:
	/** If command line override -OverrideMultiWorkerSettingsClass is set then return the specified class from the command line.
	 * Else if bForceNonEditorSettings is set, return the MultiWorkerSettingsClass.
	 * Else if the EditorMultiWorkerSettingsOverride is set and we are in the Editor, return the EditorMultiWorkerSettings.
	 * Else if multi-worker is disabled in the editor, return the single worker settings class
	 * Else if the MultiWorkerSettingsClass is set return it.
	 * Otherwise return the single worker settings class.  */
	TSubclassOf<USpatialMultiWorkerSettings> GetMultiWorkerSettingsClass(bool bForceNonEditorSettings = false);

#if WITH_EDITORONLY_DATA
	/** Defines how Unreal Editor will run the Tests in this map, without changing current Settings. */
	UPROPERTY(EditAnywhere, Category = "Testing")
	FMapTestingSettings TestingSettings;

	UPROPERTY(EditAnywhere, Category = "Testing")
	bool bEnableDebugInterface = false;
#endif

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	static void EditorRefreshSpatialDebugger();
	static void OverrideMultiWorker(const bool bDisable);
#endif // WITH_EDITOR

	/** Is multi-worker enabled in the editor*/
	bool IsMultiWorkerEnabled() const;

private:

#if WITH_EDITOR
	/** Editor override for multi worker*/
	static bool bDisableMultiWorker;
#endif // WITH_EDITOR

	/** Specify the load balancing strategy to be used for multiple workers */
	UPROPERTY(EditAnywhere, Category = "Multi-Worker")
	TSubclassOf<USpatialMultiWorkerSettings> MultiWorkerSettingsClass;

	/** Editor override to specify a different load balancing strategy to run in-editor */
	UPROPERTY(EditAnywhere, Category = "Multi-Worker")
	TSubclassOf<USpatialMultiWorkerSettings> EditorMultiWorkerSettingsOverride;

	/** Gets MultiWorkerSettingsClass if set, otherwise returns a single worker behaviour. */
	TSubclassOf<USpatialMultiWorkerSettings> GetValidWorkerSettings() const;
};
