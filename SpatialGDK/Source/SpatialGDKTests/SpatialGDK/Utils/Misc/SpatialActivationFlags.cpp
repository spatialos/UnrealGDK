// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CoreMinimal.h"

#include "TestDefinitions.h"
#include "Tests/AutomationCommon.h"
#include "Runtime/EngineSettings/Public/EngineSettings.h"

static const bool s_earliestFlag =  GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking();

FString const s_earliestFlagReport = "Spatial activation Flag [Earliest]:";
FString const s_settingsFlagReport = "Spatial activation Flag [Settings]:";
FString const s_currentFlagReport = "Spatial activation Flag [Current]:";

GDK_TEST(Core, UGeneralProjectSettings, SpatialActivationReport)
{
	UBoolProperty* spatialFlagProperty = Cast<UBoolProperty>(UGeneralProjectSettings::StaticClass()->FindPropertyByName("bSpatialNetworking"));
	TestNotNull("Property existence", spatialFlagProperty);

	UGeneralProjectSettings const* projectSettings = GetDefault<UGeneralProjectSettings>();
	TestNotNull("Settings existence", projectSettings);

	// Bypass C++ function to get the property saved to settings.
	bool savedPropertyValue = spatialFlagProperty->GetPropertyValue(spatialFlagProperty->ContainerPtrToValuePtr<void const*>(projectSettings));

	UE_LOG(LogTemp, Display, TEXT("%s %i"), s_earliestFlagReport.GetCharArray().GetData(), s_earliestFlag);
	UE_LOG(LogTemp, Display, TEXT("%s %i"), s_settingsFlagReport.GetCharArray().GetData(), savedPropertyValue);
	UE_LOG(LogTemp, Display, TEXT("%s %i"), s_currentFlagReport.GetCharArray().GetData(), projectSettings->UsesSpatialNetworking());

	return true;
}

namespace
{
	struct ReportedFlags
	{
		bool earliestFlag;
		bool settingsFlag;
		bool currentFlag;
	};

	ReportedFlags RunSubProcessAndExtractFlags(FAutomationTestBase& test, FString const& commandLineArgs)
	{
		ReportedFlags flags;

		int32 returnCode = 1;
		FString stdOut;
		FString stdErr;

		FPlatformProcess::ExecProcess(TEXT("UE4Editor"), commandLineArgs.GetCharArray().GetData(), &returnCode, &stdOut, &stdErr);

		test.TestTrue("Sucessful run", returnCode == 0);

		auto extractFlag = [&](FString const& pattern, bool& flag)
		{
			int32 patternPos = stdOut.Find(pattern);
			test.TestTrue((TEXT("Found pattern : ") + pattern).GetCharArray().GetData(),  patternPos >= 0);
			flag = FCString::Atoi(&stdOut[patternPos + pattern.Len() + 1]) != 0;
		};

		extractFlag(s_earliestFlagReport, flags.earliestFlag);
		extractFlag(s_settingsFlagReport, flags.settingsFlag);
		extractFlag(s_currentFlagReport, flags.currentFlag);

		return flags;
	}
}

GDK_TEST(Core, UGeneralProjectSettings, SpatialActivationOverride)
{
	FString projectPath = FPaths::GetProjectFilePath();
	FString commandLineArgs = projectPath;
	commandLineArgs.Append(" -ExecCmds=\"Automation RunTests SpatialGDK.Core.UGeneralProjectSettings.SpatialActivationReport; Quit\"");
	commandLineArgs.Append(" -TestExit=\"Automation Test Queue Empty\"");
	commandLineArgs.Append(" -nopause");
	commandLineArgs.Append(" -nosplash");
	commandLineArgs.Append(" -unattended");
	commandLineArgs.Append(" -nullRHI");
	commandLineArgs.Append(" -stdout");

	UBoolProperty* spatialFlagProperty = Cast<UBoolProperty>(UGeneralProjectSettings::StaticClass()->FindPropertyByName("bSpatialNetworking"));
	TestNotNull("Property existence", spatialFlagProperty);

	UGeneralProjectSettings* projectSettings = GetMutableDefault<UGeneralProjectSettings>();
	TestNotNull("Settings existence", projectSettings);

	void* spatialFlagPtr = spatialFlagProperty->ContainerPtrToValuePtr<void const*>(projectSettings);

	bool cachedFlagValue = spatialFlagProperty->GetPropertyValue(spatialFlagPtr);

	{
		projectSettings->SetUseSpatialNetworking(false);
		projectSettings->UpdateSinglePropertyInConfigFile(spatialFlagProperty, projectSettings->GetDefaultConfigFilename());

		auto flags = RunSubProcessAndExtractFlags(*this, commandLineArgs);

		TestTrue("Settings applied", flags.earliestFlag == false && flags.settingsFlag == false);
		TestTrue("Expected early value", flags.currentFlag == flags.earliestFlag);
	}

	{
		projectSettings->SetUseSpatialNetworking(true);
		projectSettings->UpdateSinglePropertyInConfigFile(spatialFlagProperty, projectSettings->GetDefaultConfigFilename());

		auto flags = RunSubProcessAndExtractFlags(*this, commandLineArgs);

		TestTrue("Settings applied", flags.earliestFlag == true && flags.settingsFlag == true);
		TestTrue("Expected early value", flags.currentFlag == flags.earliestFlag);
	}

	{
		spatialFlagProperty->SetPropertyValue(spatialFlagPtr, false);
		projectSettings->UpdateSinglePropertyInConfigFile(spatialFlagProperty, projectSettings->GetDefaultConfigFilename());

		FString commandLineOverride = commandLineArgs;
		commandLineOverride.Append(" -OverrideSpatialNetworking=true");

		auto flags = RunSubProcessAndExtractFlags(*this, commandLineOverride);

		TestTrue("Override applied", flags.earliestFlag == true && flags.settingsFlag == false);
		TestTrue("Expected early value", flags.currentFlag == flags.earliestFlag);
	}

	{
		spatialFlagProperty->SetPropertyValue(spatialFlagPtr, true);
		projectSettings->UpdateSinglePropertyInConfigFile(spatialFlagProperty, projectSettings->GetDefaultConfigFilename());

		FString commandLineOverride = commandLineArgs;
		commandLineOverride.Append(" -OverrideSpatialNetworking=false");

		auto flags = RunSubProcessAndExtractFlags(*this, commandLineOverride);

		TestTrue("Override applied", flags.earliestFlag == false && flags.settingsFlag == true);
		TestTrue("Expected early value", flags.currentFlag == flags.earliestFlag);

	}

	return true;
}
