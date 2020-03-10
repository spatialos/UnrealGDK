#include "Utils/LaunchConfigEditor.h"

#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "SlateApplication.h"
#include "SpatialGDKDefaultLaunchConfigGenerator.h"

void ULaunchConfigurationEditor::SaveConfiguration()
{
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
		GenerateDefaultLaunchConfig(Filenames[0], &LaunchConfig);
	}
}
