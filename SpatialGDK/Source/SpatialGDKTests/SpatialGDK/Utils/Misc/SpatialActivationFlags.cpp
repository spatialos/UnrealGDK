// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CoreMinimal.h"

#include "Tests/TestDefinitions.h"
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

struct SpatialActivationFlagTestFixture
{
	SpatialActivationFlagTestFixture(FAutomationTestBase& Test)
	{
		ProjectPath = FPaths::GetProjectFilePath();
		CommandLineArgs = ProjectPath;
		CommandLineArgs.Append(TEXT(" -ExecCmds=\"Automation RunTests SpatialGDK.Core.UGeneralProjectSettings.SpatialActivationReport; Quit\""));
		CommandLineArgs.Append(TEXT(" -TestExit=\"Automation Test Queue Empty\""));
		CommandLineArgs.Append(TEXT(" -nopause"));
		CommandLineArgs.Append(TEXT(" -nosplash"));
		CommandLineArgs.Append(TEXT(" -unattended"));
		CommandLineArgs.Append(TEXT(" -nullRHI"));
		CommandLineArgs.Append(TEXT(" -stdout"));

		SpatialFlagProperty = Cast<UBoolProperty>(UGeneralProjectSettings::StaticClass()->FindPropertyByName("bSpatialNetworking"));
		Test.TestNotNull("Property existence", SpatialFlagProperty);

		ProjectSettings = GetMutableDefault<UGeneralProjectSettings>();
		Test.TestNotNull("Settings existence", ProjectSettings);

		SpatialFlagPtr = SpatialFlagProperty->ContainerPtrToValuePtr<const void*>(ProjectSettings);
		bSavedFlagValue = SpatialFlagProperty->GetPropertyValue(SpatialFlagPtr);
	}

	~SpatialActivationFlagTestFixture()
	{
		ProjectSettings->SetUsesSpatialNetworking(bSavedFlagValue);
		ProjectSettings->UpdateSinglePropertyInConfigFile(SpatialFlagProperty, ProjectSettings->GetDefaultConfigFilename());
	}

	void ChangeSetting(bool bEnabled)
	{
		ProjectSettings->SetUsesSpatialNetworking(bEnabled);
		ProjectSettings->UpdateSinglePropertyInConfigFile(SpatialFlagProperty, ProjectSettings->GetDefaultConfigFilename());
	}

	FString CommandLineArgs;

private:
	FString ProjectPath;
	UBoolProperty* SpatialFlagProperty;
	UGeneralProjectSettings* ProjectSettings;
	void* SpatialFlagPtr;
	bool bSavedFlagValue;
};

GDK_TEST(Core, UGeneralProjectSettings, SpatialActivationSetting_False)
{
	SpatialActivationFlagTestFixture TestFixture(*this);

	TestFixture.ChangeSetting(false);

	ReportedFlags Flags = RunSubProcessAndExtractFlags(*this, TestFixture.CommandLineArgs);

	TestTrue("Settings applied", Flags.bEarliestFlag == false);
	TestTrue("Expected early value", Flags.bCurrentFlag == Flags.bEarliestFlag);

	return true;
}

GDK_TEST(Core, UGeneralProjectSettings, SpatialActivationSetting_True)
{
	SpatialActivationFlagTestFixture TestFixture(*this);

	TestFixture.ChangeSetting(true);

	ReportedFlags Flags = RunSubProcessAndExtractFlags(*this, TestFixture.CommandLineArgs);

	TestTrue("Settings applied", Flags.bEarliestFlag == true);
	TestTrue("Expected early value", Flags.bCurrentFlag == Flags.bEarliestFlag);

	return true;
}

GDK_TEST(Core, UGeneralProjectSettings, SpatialActivationOverride_True)
{
	SpatialActivationFlagTestFixture TestFixture(*this);

	TestFixture.ChangeSetting(false);

	FString CommandLineOverride = TestFixture.CommandLineArgs;
	CommandLineOverride.Append(" -OverrideSpatialNetworking=true");

	ReportedFlags Flags = RunSubProcessAndExtractFlags(*this, CommandLineOverride);

	TestTrue("Override applied", Flags.bEarliestFlag == true);
	TestTrue("Expected early value", Flags.bCurrentFlag == Flags.bEarliestFlag);

	return true;
}

GDK_TEST(Core, UGeneralProjectSettings, SpatialActivationOverride_False)
{
	SpatialActivationFlagTestFixture TestFixture(*this);

	TestFixture.ChangeSetting(true);

	FString CommandLineOverride = TestFixture.CommandLineArgs;
	CommandLineOverride.Append(" -OverrideSpatialNetworking=false");

	ReportedFlags Flags = RunSubProcessAndExtractFlags(*this, CommandLineOverride);

	TestTrue("Override applied", Flags.bEarliestFlag == false);
	TestTrue("Expected early value", Flags.bCurrentFlag == Flags.bEarliestFlag);

	return true;
}
