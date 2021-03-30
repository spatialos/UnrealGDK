// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CoreMinimal.h"

#include "Runtime/EngineSettings/Classes/GeneralProjectSettings.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestDefinitions.h"

#include "Utils/GDKPropertyMacros.h"

namespace
{
bool bEarliestFlag;

const FString EarliestFlagReport = TEXT("Spatial activation Flag [Earliest]:");
const FString CurrentFlagReport = TEXT("Spatial activation Flag [Current]:");
} // namespace

void InitializeSpatialFlagEarlyValues()
{
	bEarliestFlag = GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking();
}

GDK_SLOW_TEST(Core, UGeneralProjectSettings, SpatialActivationReport)
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

	Test.TestTrue("Successful run", ReturnCode == 0);

	auto ExtractFlag = [&](const FString& Pattern, bool& bFlag) {
		int32 PatternPos = StdOut.Find(Pattern);
		Test.TestTrue(*(TEXT("Found pattern : ") + Pattern), PatternPos >= 0);
		bFlag = FCString::Atoi(&StdOut[PatternPos + Pattern.Len() + 1]) != 0;
	};

	ExtractFlag(EarliestFlagReport, Flags.bEarliestFlag);
	ExtractFlag(CurrentFlagReport, Flags.bCurrentFlag);

	return Flags;
}
} // namespace

struct SpatialActivationFlagTestFixture
{
	SpatialActivationFlagTestFixture(FAutomationTestBase& Test)
	{
		ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
		CommandLineArgs = ProjectPath;
		CommandLineArgs.Append(
			TEXT(" -ExecCmds=\"Automation RunTests SpatialGDKSlow.Core.UGeneralProjectSettings.SpatialActivationReport; Quit\""));
		CommandLineArgs.Append(TEXT(" -TestExit=\"Automation Test Queue Empty\""));
		CommandLineArgs.Append(TEXT(" -nopause"));
		CommandLineArgs.Append(TEXT(" -nosplash"));
		CommandLineArgs.Append(TEXT(" -unattended"));
		CommandLineArgs.Append(TEXT(" -nullRHI"));
		CommandLineArgs.Append(TEXT(" -stdout"));

		SpatialFlagProperty =
			GDK_CASTFIELD<GDK_PROPERTY(BoolProperty)>(UGeneralProjectSettings::StaticClass()->FindPropertyByName("bSpatialNetworking"));
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
	TFuture<ReportedFlags> CheckResult;

private:
	FString ProjectPath;
	GDK_PROPERTY(BoolProperty) * SpatialFlagProperty;
	UGeneralProjectSettings* ProjectSettings;
	void* SpatialFlagPtr;
	bool bSavedFlagValue;
};

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FRunSubProcessCommand, FAutomationTestBase*, Test,
												 TSharedPtr<SpatialActivationFlagTestFixture>, Fixture, bool, ExpectedValue);
bool FRunSubProcessCommand::Update()
{
	if (!Fixture->CheckResult.IsValid())
	{
		Fixture->CheckResult = Async(EAsyncExecution::Thread, TFunction<ReportedFlags()>([&] {
										 return RunSubProcessAndExtractFlags(*Test, Fixture->CommandLineArgs);
									 }));
	}

	if (!Fixture->CheckResult.IsReady())
	{
		return false;
	}
	ReportedFlags Flags = Fixture->CheckResult.Get();

	Test->TestTrue("Settings applied", Flags.bEarliestFlag == ExpectedValue);
	Test->TestTrue("Expected early value", Flags.bCurrentFlag == Flags.bEarliestFlag);

	return true;
}

GDK_SLOW_TEST(Core, UGeneralProjectSettings, SpatialActivationSetting_False)
{
	auto TestFixture = MakeShared<SpatialActivationFlagTestFixture>(*this);
	TestFixture->ChangeSetting(false);

	ADD_LATENT_AUTOMATION_COMMAND(FRunSubProcessCommand(this, TestFixture, false));

	return true;
}

GDK_SLOW_TEST(Core, UGeneralProjectSettings, SpatialActivationSetting_True)
{
	auto TestFixture = MakeShared<SpatialActivationFlagTestFixture>(*this);
	TestFixture->ChangeSetting(true);

	ADD_LATENT_AUTOMATION_COMMAND(FRunSubProcessCommand(this, TestFixture, true));

	return true;
}

GDK_SLOW_TEST(Core, UGeneralProjectSettings, SpatialActivationOverride_True)
{
	auto TestFixture = MakeShared<SpatialActivationFlagTestFixture>(*this);
	TestFixture->ChangeSetting(false);

	FString CommandLineOverride = TestFixture->CommandLineArgs;
	CommandLineOverride.Append(" -OverrideSpatialNetworking=true");
	TestFixture->CommandLineArgs = CommandLineOverride;

	ADD_LATENT_AUTOMATION_COMMAND(FRunSubProcessCommand(this, TestFixture, true));

	return true;
}

GDK_SLOW_TEST(Core, UGeneralProjectSettings, SpatialActivationOverride_False)
{
	auto TestFixture = MakeShared<SpatialActivationFlagTestFixture>(*this);
	TestFixture->ChangeSetting(false);

	FString CommandLineOverride = TestFixture->CommandLineArgs;
	CommandLineOverride.Append(" -OverrideSpatialNetworking=false");
	TestFixture->CommandLineArgs = CommandLineOverride;

	ADD_LATENT_AUTOMATION_COMMAND(FRunSubProcessCommand(this, TestFixture, false));

	return true;
}
