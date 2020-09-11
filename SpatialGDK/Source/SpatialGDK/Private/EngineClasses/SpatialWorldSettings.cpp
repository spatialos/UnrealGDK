// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "EngineClasses/SpatialWorldSettings.h"

#include "EngineUtils.h"
#include "SpatialGDKSettings.h"
#include "Utils/SpatialDebugger.h"

ASpatialWorldSettings::ASpatialWorldSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MultiWorkerSettingsClass(nullptr)
	, EditorMultiWorkerSettingsOverride(nullptr)
{
}

TSubclassOf<USpatialMultiWorkerSettings> ASpatialWorldSettings::GetMultiWorkerSettingsClass(bool bForceNonEditorSettings /*= false*/)
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();

	if (SpatialGDKSettings->OverrideMultiWorkerSettingsClass.IsSet())
	{
		// If command line override for Multi Worker Settings is set then use the specified Multi Worker Settings class.
		FString OverrideMultiWorkerSettingsClass = SpatialGDKSettings->OverrideMultiWorkerSettingsClass.GetValue();
		FSoftClassPath MultiWorkerSettingsSoftClassPath(OverrideMultiWorkerSettingsClass);
		MultiWorkerSettingsClass = MultiWorkerSettingsSoftClassPath.TryLoadClass<USpatialMultiWorkerSettings>();
		checkf(MultiWorkerSettingsClass != nullptr, TEXT("%s is not a valid class"), *OverrideMultiWorkerSettingsClass);
		return GetValidWorkerSettings();
	}
	else if (bForceNonEditorSettings && MultiWorkerSettingsClass != nullptr)
	{
		// If bForceNonEditorSettings is set and the multi worker setting class is set use the multi worker settings.
		return MultiWorkerSettingsClass;
	}
	else if (bForceNonEditorSettings)
	{
		// If bForceNonEditorSettings is set and no multi worker settings class is set always return a valid class (use single worker
		// behaviour).
		return USpatialMultiWorkerSettings::StaticClass();
	}
	else if (!IsMultiWorkerEnabled())
	{
		// If multi worker is disabled in editor, use the single worker behaviour.
		return USpatialMultiWorkerSettings::StaticClass();
	}
#if WITH_EDITOR
	else if (EditorMultiWorkerSettingsOverride != nullptr)
	{
		// If the editor override Multi Worker Settings is set and we are in the Editor use the Editor Multi Worker Settings.
		return EditorMultiWorkerSettingsOverride;
	}
#endif // WITH_EDITOR
	return GetValidWorkerSettings();
}

TSubclassOf<USpatialMultiWorkerSettings> ASpatialWorldSettings::GetValidWorkerSettings() const
{
	if (MultiWorkerSettingsClass != nullptr)
	{
		// If the MultiWorkerSettingsClass is set, return it.
		return MultiWorkerSettingsClass;
	}
	else
	{
		// Otherwise, return a valid class, return single worker settings class.
		return USpatialMultiWorkerSettings::StaticClass();
	}
}

#if WITH_EDITOR
void ASpatialWorldSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property != nullptr)
	{
		const FName PropertyName(PropertyChangedEvent.Property->GetFName());
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ASpatialWorldSettings, MultiWorkerSettingsClass)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ASpatialWorldSettings, EditorMultiWorkerSettingsOverride))
		{
			EditorRefreshSpatialDebugger();
		}
	}
}

void ASpatialWorldSettings::EditorRefreshSpatialDebugger()
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

bool ASpatialWorldSettings::IsMultiWorkerEnabled() const
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();

	// Check if multi-worker settings class was overridden from the command line
	if (SpatialGDKSettings->OverrideMultiWorkerSettingsClass.IsSet())
	{
		// If command line override for Multi Worker Settings is set then enable multi-worker.
		return true;
	}
#if WITH_EDITOR
	else if (SpatialGDKSettings->IsDisabledMultiWorkerEditor())
	{
		// If editor for Multi Worker Settings is set then disable multi-worker.
		return false;
	}
#endif // WITH_EDITOR
	return true;
}
