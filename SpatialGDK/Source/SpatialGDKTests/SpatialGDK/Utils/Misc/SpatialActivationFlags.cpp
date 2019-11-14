// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CoreMinimal.h"

#include "TestDefinitions.h"
#include "Tests/AutomationCommon.h"
#include "Runtime/EngineSettings/Public/EngineSettings.h"

namespace
{
	bool bEarliestFlag;

	const FString EarliestFlagReport = TEXT("Spatial activation Flag [Earliest]:");
	const FString CurrentFlagReport = TEXT("Spatial activation Flag [Current]:");
}

void InitializeSpatialFlagEarlyValues()
{
	bEarliestFlag = GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking();
}

GDK_TEST(Core, UGeneralProjectSettings, SpatialActivationReport)
{
	const UGeneralProjectSettings* ProjectSettings = GetDefault<UGeneralProjectSettings>();

	UE_LOG(LogTemp, Display, TEXT("%s %i"), *EarliestFlagReport, bEarliestFlag);
	UE_LOG(LogTemp, Display, TEXT("%s %i"), *CurrentFlagReport, ProjectSettings->UsesSpatialNetworking());

	return true;
}

namespace
{
	struct ReportedFlags
	{
		bool bEarliestFlag;
		bool bCurrentFlag;
	};

	ReportedFlags RunSubProcessAndExtractFlags(FAutomationTestBase& Test, const FString& CommandLineArgs)
	{
		ReportedFlags Flags;

		int32 ReturnCode = 1;
		FString StdOut;
		FString StdErr;

		FPlatformProcess::ExecProcess(TEXT("UE4Editor"), *CommandLineArgs, &ReturnCode, &StdOut, &StdErr);

		Test.TestTrue("Sucessful run", ReturnCode == 0);

		auto ExtractFlag = [&](const FString& Pattern, bool& bFlag)
		{
			int32 PatternPos = StdOut.Find(Pattern);
			Test.TestTrue(*(TEXT("Found pattern : ") + Pattern),  PatternPos >= 0);
			bFlag = FCString::Atoi(&StdOut[PatternPos + Pattern.Len() + 1]) != 0;
		};

		ExtractFlag(EarliestFlagReport, Flags.bEarliestFlag);
		ExtractFlag(CurrentFlagReport, Flags.bCurrentFlag);

		return Flags;
	}
}

GDK_TEST(Core, UGeneralProjectSettings, SpatialActivationOverride)
{
#if 0
	FString ProjectPath = FPaths::GetProjectFilePath();
	FString CommandLineArgs = ProjectPath;
	CommandLineArgs.Append(TEXT(" -ExecCmds=\"Automation RunTests SpatialGDK.Core.UGeneralProjectSettings.SpatialActivationReport; Quit\""));
	CommandLineArgs.Append(TEXT(" -TestExit=\"Automation Test Queue Empty\""));
	CommandLineArgs.Append(TEXT(" -nopause"));
	CommandLineArgs.Append(TEXT(" -nosplash"));
	CommandLineArgs.Append(TEXT(" -unattended"));
	CommandLineArgs.Append(TEXT(" -nullRHI"));
	CommandLineArgs.Append(TEXT(" -stdout"));

	UBoolProperty* SpatialFlagProperty = Cast<UBoolProperty>(UGeneralProjectSettings::StaticClass()->FindPropertyByName("bSpatialNetworking"));
	TestNotNull("Property existence", SpatialFlagProperty);

	UGeneralProjectSettings* ProjectSettings = GetMutableDefault<UGeneralProjectSettings>();
	TestNotNull("Settings existence", ProjectSettings);

	void* SpatialFlagPtr = SpatialFlagProperty->ContainerPtrToValuePtr<const void*>(ProjectSettings);

	bool bSavedFlagValue = SpatialFlagProperty->GetPropertyValue(SpatialFlagPtr);

	{
		ProjectSettings->SetUsesSpatialNetworking(false);
		ProjectSettings->UpdateSinglePropertyInConfigFile(SpatialFlagProperty, ProjectSettings->GetDefaultConfigFilename());

		ReportedFlags Flags = RunSubProcessAndExtractFlags(*this, CommandLineArgs);

		TestTrue("Settings applied", Flags.bEarliestFlag == false);
		TestTrue("Expected early value", Flags.bCurrentFlag == Flags.bEarliestFlag);
	}

	{
		ProjectSettings->SetUsesSpatialNetworking(true);
		ProjectSettings->UpdateSinglePropertyInConfigFile(SpatialFlagProperty, ProjectSettings->GetDefaultConfigFilename());

		ReportedFlags Flags = RunSubProcessAndExtractFlags(*this, CommandLineArgs);

		TestTrue("Settings applied", Flags.bEarliestFlag == true);
		TestTrue("Expected early value", Flags.bCurrentFlag == Flags.bEarliestFlag);
	}

	{
		SpatialFlagProperty->SetPropertyValue(SpatialFlagPtr, false);
		ProjectSettings->UpdateSinglePropertyInConfigFile(SpatialFlagProperty, ProjectSettings->GetDefaultConfigFilename());

		FString CommandLineOverride = CommandLineArgs;
		CommandLineOverride.Append(" -OverrideSpatialNetworking=true");

		ReportedFlags Flags = RunSubProcessAndExtractFlags(*this, CommandLineOverride);

		TestTrue("Override applied", Flags.bEarliestFlag == true);
		TestTrue("Expected early value", Flags.bCurrentFlag == Flags.bEarliestFlag);
	}

	{
		SpatialFlagProperty->SetPropertyValue(SpatialFlagPtr, true);
		ProjectSettings->UpdateSinglePropertyInConfigFile(SpatialFlagProperty, ProjectSettings->GetDefaultConfigFilename());

		FString CommandLineOverride = CommandLineArgs;
		CommandLineOverride.Append(" -OverrideSpatialNetworking=false");

		ReportedFlags Flags = RunSubProcessAndExtractFlags(*this, CommandLineOverride);

		TestTrue("Override applied", Flags.bEarliestFlag == false);
		TestTrue("Expected early value", Flags.bCurrentFlag == Flags.bEarliestFlag);

	}

	// Restore original flags
	ProjectSettings->SetUsesSpatialNetworking(bSavedFlagValue);
	ProjectSettings->UpdateSinglePropertyInConfigFile(SpatialFlagProperty, ProjectSettings->GetDefaultConfigFilename());
#endif
	return true;
}
