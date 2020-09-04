// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "EngineClasses/SpatialWorldSettings.h"

#include "EngineUtils.h"
#include "SpatialGDKSettings.h"
#include "Utils/SpatialDebugger.h"

ASpatialWorldSettings::ASpatialWorldSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bEnableMultiWorker(false)
	, MultiWorkerSettingsClass(nullptr)
	, EditorMultiWorkerSettingsOverride(nullptr)
{	
}

 void ASpatialWorldSettings::PostLoad()
 {
	 Super::PostLoad();

	 // Check for command line overrides
	 const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();

	 if (SpatialGDKSettings->OverrideMultiWorkerSettingsClass.IsSet())
	 {
		 UE_LOG(LogTemp, Warning, TEXT("GetMultiWorkerSettingsClass case 1"));

		 // If command line override for Multi Worker Settings is set then use the specified Multi Worker Settings class.
		 FString OverrideMultiWorkerSettingsClass = SpatialGDKSettings->OverrideMultiWorkerSettingsClass.GetValue();
		 FSoftClassPath MultiWorkerSettingsSoftClassPath(OverrideMultiWorkerSettingsClass);
		 TSubclassOf<USpatialMultiWorkerSettings> CommandLineMultiWorkerSettingsOverride =
			 MultiWorkerSettingsSoftClassPath.TryLoadClass<USpatialMultiWorkerSettings>();
		 checkf(CommandLineMultiWorkerSettingsOverride != nullptr, TEXT("%s is not a valid class"), *OverrideMultiWorkerSettingsClass);
		 UE_LOG(LogTemp, Warning, TEXT("Using class: %s"), *OverrideMultiWorkerSettingsClass);
		 MultiWorkerSettingsClass = CommandLineMultiWorkerSettingsOverride;
		 bEnableMultiWorker = true;
	 }
	 else if (SpatialGDKSettings->bOverrideMultiWorker.IsSet() && SpatialGDKSettings->bOverrideMultiWorker.GetValue())
	 {
		 // If enable multi-worker was overridden from the command line update it 
		 bEnableMultiWorker = false;
	 }
 }

TSubclassOf<USpatialMultiWorkerSettings> ASpatialWorldSettings::GetMultiWorkerSettingsClass(bool bForceNonEditorSettings /*= false*/) const
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();

	if (SpatialGDKSettings->OverrideMultiWorkerSettingsClass.IsSet())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetMultiWorkerSettingsClass case 1"));

		return GetValidWorkerSettings();
	}
	else if (!IsMultiWorkerEnabled())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetMultiWorkerSettingsClass case 2"));

		// If multi worker is disabled, use the single worker behaviour.
		return USpatialMultiWorkerSettings::StaticClass();
	}
	else if (bForceNonEditorSettings && MultiWorkerSettingsClass != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetMultiWorkerSettingsClass case 3"));
		// If bForceNonEditorSettings is set and the multi worker setting class is set use the multi worker settings.
		return MultiWorkerSettingsClass;
	}
	else if (bForceNonEditorSettings)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetMultiWorkerSettingsClass case 4"));
		// If bForceNonEditorSettings is set and no multi worker settings class is set always return a valid class (use single worker
		// behaviour).
		return USpatialMultiWorkerSettings::StaticClass();
	}
#if WITH_EDITOR
	else if (EditorMultiWorkerSettingsOverride != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetMultiWorkerSettingsClass case 5"));
		// If the editor override Multi Worker Settings is set and we are in the Editor use the Editor Multi Worker Settings.
		return EditorMultiWorkerSettingsOverride;
	}
#endif // WITH_EDITOR
	UE_LOG(LogTemp, Warning, TEXT("GetMultiWorkerSettingsClass case 6"));
	return GetValidWorkerSettings();
}


TSubclassOf<USpatialMultiWorkerSettings> ASpatialWorldSettings::GetValidWorkerSettings() const
{
	if (MultiWorkerSettingsClass != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetValidWorkerSettings case A"));
		// If the MultiWorkerSettingsClass is set, return it.
		return MultiWorkerSettingsClass;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GetValidWorkerSettings case B"));
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
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ASpatialWorldSettings, EditorMultiWorkerSettingsOverride)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ASpatialWorldSettings, bEnableMultiWorker))
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
	// Check if multi-worker settings class was overridden from the command line
	return bEnableMultiWorker;
}
