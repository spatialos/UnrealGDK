// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/LaunchConfigEditor.h"

#include "SpatialGDKSettings.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKDefaultLaunchConfigGenerator.h"
#include "SpatialRuntimeLoadBalancingStrategies.h"

#include "DesktopPlatformModule.h"
#include "Framework/Application/SlateApplication.h"
#include "IDesktopPlatform.h"

void ULaunchConfigurationEditor::PostInitProperties()
{
	Super::PostInitProperties();

	const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>();

	LaunchConfiguration = SpatialGDKEditorSettings->LaunchConfigDesc;
	FillWorkerConfigurationFromCurrentMap(LaunchConfiguration.ServerWorkersMap, LaunchConfiguration.World.Dimensions);
}

void ULaunchConfigurationEditor::OnWorkerTypesChanged()
{
	LaunchConfiguration.OnWorkerTypesChanged();
	for (TPair<FName, FWorkerTypeLaunchSection>& LaunchSection : LaunchConfiguration.ServerWorkersMap)
	{
		if (LaunchSection.Value.WorkerLoadBalancing == nullptr)
		{
			LaunchSection.Value.WorkerLoadBalancing = USingleWorkerRuntimeStrategy::StaticClass()->GetDefaultObject<UAbstractRuntimeLoadBalancingStrategy>();
		}
	}
	PostEditChange();
}

void ULaunchConfigurationEditor::SaveConfiguration()
{
	if (!ValidateGeneratedLaunchConfig(LaunchConfiguration, LaunchConfiguration.ServerWorkersMap))
	{
		return;
	}

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	FString DefaultOutPath = SpatialGDKServicesConstants::SpatialOSDirectory;
	TArray<FString> Filenames;

	bool bSaved = DesktopPlatform->SaveFileDialog(
		FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
		TEXT("Save launch configuration"),
		DefaultOutPath,
		TEXT(""),
		TEXT("JSON Configuration|*.json"),
		EFileDialogFlags::None,
		Filenames);

	if (bSaved && Filenames.Num() > 0)
	{
		if (GenerateLaunchConfig(Filenames[0], &LaunchConfiguration, LaunchConfiguration.ServerWorkersMap))
		{
			OnConfigurationSaved.ExecuteIfBound(this, Filenames[0]);
		}
	}
}
