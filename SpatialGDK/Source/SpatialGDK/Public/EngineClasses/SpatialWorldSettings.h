// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineUtils.h"
#include "LoadBalancing/SpatialMultiWorkerSettings.h"
#include "SpatialGDKSettings.h"
#include "Utils/LayerInfo.h"
#include "Utils/SpatialDebugger.h"
#include "Utils/SpatialStatics.h"

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
	GENERATED_BODY()
	friend class USpatialStatics;

private:
	/** Enable running different server worker types to split the simulation. */
	UPROPERTY(EditAnywhere, Config, Category = "Multi-Worker")
	bool bEnableMultiWorker;

	UPROPERTY(EditAnywhere, Category = "Multi-Worker", meta = (EditCondition = "bEnableMultiWorker"))
	TSubclassOf<USpatialMultiWorkerSettings> MultiWorkerSettingsClass;

	UPROPERTY(EditAnywhere, Category = "Multi-Worker", meta = (EditCondition = "bEnableMultiWorker"))
	TSubclassOf<USpatialMultiWorkerSettings> EditorMultiWorkerSettingsOverride;

public:
	/** If command line override for Multi Worker Settings Class is set then use the specified Multi Worker Settings class.
	* Else if multi worker is disabled, use the single worker behaviour.
	* Else if bForceNonEditorSettings is set use the Multi Worker Settings class.
	* Else if the Editor Multi Worker Settings Override is set and we are in the Editor use the Editor Multi Worker Settings.
	* Else if the Multi Worker Settings Class is set use it.
	* Otherwise default to single worker behaviour.  */
	TSubclassOf<USpatialMultiWorkerSettings> GetMultiWorkerSettingsClass(bool bForceNonEditorSettings = false) const
	{
		const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();

		if (SpatialGDKSettings->OverrideMultiWorkerSettingsClass.IsSet())
		{
			// If command line override for Multi Worker Settings is set then use the specified Multi Worker Settings class.
			FString OverrideMultiWorkerSettingsClass = SpatialGDKSettings->OverrideMultiWorkerSettingsClass.GetValue();
			FSoftClassPath MultiWorkerSettingsSoftClassPath(OverrideMultiWorkerSettingsClass);
			TSubclassOf<USpatialMultiWorkerSettings> CommandLineMultiWorkerSettingsOverride =
				MultiWorkerSettingsSoftClassPath.TryLoadClass<USpatialMultiWorkerSettings>();
			checkf(CommandLineMultiWorkerSettingsOverride != nullptr, TEXT("%s is not a valid class"), *OverrideMultiWorkerSettingsClass);
			return CommandLineMultiWorkerSettingsOverride;
		}
		else if (!IsMultiWorkerEnabledInWorldSettings())
		{
			// If multi worker is disabled, use the single worker behaviour.
			return USpatialMultiWorkerSettings::StaticClass();
		}
		else if (bForceNonEditorSettings && MultiWorkerSettingsClass != nullptr)
		{
			// If bForceNonEditorSettings is set and the multi worker setting class is set use the multi worker settings.
			return MultiWorkerSettingsClass;
		}
		else if (bForceNonEditorSettings)
		{
			// If bForceNonEditorSettings is set and no multi worker settings class is set always return a valid class (use single worker behaviour).
			return USpatialMultiWorkerSettings::StaticClass();
		}
#if WITH_EDITOR
		else if (EditorMultiWorkerSettingsOverride != nullptr)
		{
			// If the editor override Multi Worker Settings is set and we are in the Editor use the Editor Multi Worker Settings.
			return EditorMultiWorkerSettingsOverride;
		}
#endif // WITH_EDITOR
		else if (MultiWorkerSettingsClass != nullptr)
		{
			// If the multi worker setting class is set use the multi worker settings.
			return MultiWorkerSettingsClass;
		}
		else
		{
			// If no class is set always return a valid class, (use single worker behaviour).
			return USpatialMultiWorkerSettings::StaticClass();
		}
	}

#if WITH_EDITORONLY_DATA
	/** Defines how Unreal Editor will run the Tests in this map, without changing current Settings. */
	UPROPERTY(EditAnywhere, Category = "Testing")
	FMapTestingSettings TestingSettings;
#endif

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);

		if (PropertyChangedEvent.Property != nullptr)
		{
			const FName PropertyName(PropertyChangedEvent.Property->GetFName());
			if (PropertyName == GET_MEMBER_NAME_CHECKED(ASpatialWorldSettings, MultiWorkerSettingsClass)
				|| PropertyName == GET_MEMBER_NAME_CHECKED(ASpatialWorldSettings, EditorMultiWorkerSettingsOverride)
				|| PropertyName == GET_MEMBER_NAME_CHECKED(ASpatialWorldSettings, bEnableMultiWorker))
			{
				EditorRefreshSpatialDebugger();
			}
		}
	}

	static void EditorRefreshSpatialDebugger()
	{
		// Refresh the worker boundaries in the editor
		UWorld* World = GEditor->GetEditorWorldContext().World();
		for (TActorIterator<ASpatialDebugger> It(World); It; ++It)
		{
			ASpatialDebugger* FoundActor = *It;
			FoundActor->EditorRefreshWorkerRegions();
		}
	}
#endif // WITH_EDITOR

	bool IsMultiWorkerEnabledInWorldSettings() const
	{
		// Check if multi-worker settings class was overridden from the command line
		const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
		if (SpatialGDKSettings->OverrideMultiWorkerSettingsClass.IsSet())
		{
			return true;
		}
		// Check if multi-worker was overridden from the command line 
		else if (SpatialGDKSettings->bOverrideMultiWorker.IsSet() && SpatialGDKSettings->bOverrideMultiWorker.GetValue())
		{
			return false;
		}
		return bEnableMultiWorker;
	}
};
