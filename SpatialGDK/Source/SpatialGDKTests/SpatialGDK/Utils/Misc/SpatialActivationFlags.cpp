// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CoreMinimal.h"

#include "TestDefinitions.h"
#include "Tests/AutomationCommon.h"
#include "Runtime/EngineSettings/Public/EngineSettings.h"

namespace
{
	const bool bEarliestFlag = GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking();

	FString const EarliestFlagReport = TEXT("Spatial activation Flag [Earliest]:");
	FString const SettingsFlagReport = TEXT("Spatial activation Flag [Settings]:");
	FString const CurrentFlagReport = TEXT("Spatial activation Flag [Current]:");
}

GDK_TEST(Core, UGeneralProjectSettings, SpatialActivationReport)
{
	UBoolProperty* SpatialFlagProperty = Cast<UBoolProperty>(UGeneralProjectSettings::StaticClass()->FindPropertyByName("bSpatialNetworking"));
	TestNotNull("Property existence", SpatialFlagProperty);

	UGeneralProjectSettings const* ProjectSettings = GetDefault<UGeneralProjectSettings>();
	TestNotNull("Settings existence", ProjectSettings);

	// Bypass C++ function to get the property saved to settings.
	bool SettingsPropertyValue = SpatialFlagProperty->GetPropertyValue(SpatialFlagProperty->ContainerPtrToValuePtr<void const*>(ProjectSettings));

	UE_LOG(LogTemp, Display, TEXT("%s %i"), EarliestFlagReport.GetCharArray().GetData(), bEarliestFlag);
	UE_LOG(LogTemp, Display, TEXT("%s %i"), SettingsFlagReport.GetCharArray().GetData(), SettingsPropertyValue);
	UE_LOG(LogTemp, Display, TEXT("%s %i"), CurrentFlagReport.GetCharArray().GetData(), ProjectSettings->UsesSpatialNetworking());

	return true;
}

namespace
{
	struct ReportedFlags
	{
		bool bEarliestFlag;
		bool bSettingsFlag;
		bool bCurrentFlag;
	};

	ReportedFlags RunSubProcessAndExtractFlags(FAutomationTestBase& test, FString const& commandLineArgs)
	{
		ReportedFlags Flags;

		int32 ReturnCode = 1;
		FString StdOut;
		FString StdErr;

		FPlatformProcess::ExecProcess(TEXT("UE4Editor"), commandLineArgs.GetCharArray().GetData(), &ReturnCode, &StdOut, &StdErr);

		test.TestTrue("Sucessful run", ReturnCode == 0);

		auto extractFlag = [&](FString const& pattern, bool& flag)
		{
			int32 patternPos = StdOut.Find(pattern);
			test.TestTrue((TEXT("Found pattern : ") + pattern).GetCharArray().GetData(),  patternPos >= 0);
			flag = FCString::Atoi(&StdOut[patternPos + pattern.Len() + 1]) != 0;
		};

		extractFlag(EarliestFlagReport, Flags.bEarliestFlag);
		extractFlag(SettingsFlagReport, Flags.bSettingsFlag);
		extractFlag(CurrentFlagReport, Flags.bCurrentFlag);

		return Flags;
	}
}

GDK_TEST(Core, UGeneralProjectSettings, SpatialActivationOverride)
{
	FString ProjectPath = FPaths::GetProjectFilePath();
	FString CommandLineArgs = ProjectPath;
	CommandLineArgs.Append(" -ExecCmds=\"Automation RunTests SpatialGDK.Core.UGeneralProjectSettings.SpatialActivationReport; Quit\"");
	CommandLineArgs.Append(" -TestExit=\"Automation Test Queue Empty\"");
	CommandLineArgs.Append(" -nopause");
	CommandLineArgs.Append(" -nosplash");
	CommandLineArgs.Append(" -unattended");
	CommandLineArgs.Append(" -nullRHI");
	CommandLineArgs.Append(" -stdout");

	UBoolProperty* SpatialFlagProperty = Cast<UBoolProperty>(UGeneralProjectSettings::StaticClass()->FindPropertyByName("bSpatialNetworking"));
	TestNotNull("Property existence", SpatialFlagProperty);

	UGeneralProjectSettings* ProjectSettings = GetMutableDefault<UGeneralProjectSettings>();
	TestNotNull("Settings existence", ProjectSettings);

	void* SpatialFlagPtr = SpatialFlagProperty->ContainerPtrToValuePtr<void const*>(ProjectSettings);

	bool SavedFlagValue = SpatialFlagProperty->GetPropertyValue(SpatialFlagPtr);

	{
		ProjectSettings->SetUseSpatialNetworking(false);
		ProjectSettings->UpdateSinglePropertyInConfigFile(SpatialFlagProperty, ProjectSettings->GetDefaultConfigFilename());

		auto flags = RunSubProcessAndExtractFlags(*this, CommandLineArgs);

		TestTrue("Settings applied", flags.bEarliestFlag == false && flags.bSettingsFlag == false);
		TestTrue("Expected early value", flags.bCurrentFlag == flags.bEarliestFlag);
	}

	{
		ProjectSettings->SetUseSpatialNetworking(true);
		ProjectSettings->UpdateSinglePropertyInConfigFile(SpatialFlagProperty, ProjectSettings->GetDefaultConfigFilename());

		auto flags = RunSubProcessAndExtractFlags(*this, CommandLineArgs);

		TestTrue("Settings applied", flags.bEarliestFlag == true && flags.bSettingsFlag == true);
		TestTrue("Expected early value", flags.bCurrentFlag == flags.bEarliestFlag);
	}

	{
		SpatialFlagProperty->SetPropertyValue(SpatialFlagPtr, false);
		ProjectSettings->UpdateSinglePropertyInConfigFile(SpatialFlagProperty, ProjectSettings->GetDefaultConfigFilename());

		FString commandLineOverride = CommandLineArgs;
		commandLineOverride.Append(" -OverrideSpatialNetworking=true");

		auto flags = RunSubProcessAndExtractFlags(*this, commandLineOverride);

		TestTrue("Override applied", flags.bEarliestFlag == true && flags.bSettingsFlag == false);
		TestTrue("Expected early value", flags.bCurrentFlag == flags.bEarliestFlag);
	}

	{
		SpatialFlagProperty->SetPropertyValue(SpatialFlagPtr, true);
		ProjectSettings->UpdateSinglePropertyInConfigFile(SpatialFlagProperty, ProjectSettings->GetDefaultConfigFilename());

		FString commandLineOverride = CommandLineArgs;
		commandLineOverride.Append(" -OverrideSpatialNetworking=false");

		auto flags = RunSubProcessAndExtractFlags(*this, commandLineOverride);

		TestTrue("Override applied", flags.bEarliestFlag == false && flags.bSettingsFlag == true);
		TestTrue("Expected early value", flags.bCurrentFlag == flags.bEarliestFlag);

	}

	// Restore original flags
	ProjectSettings->SetUseSpatialNetworking(SavedFlagValue);
	ProjectSettings->UpdateSinglePropertyInConfigFile(SpatialFlagProperty, ProjectSettings->GetDefaultConfigFilename());

	return true;
}
