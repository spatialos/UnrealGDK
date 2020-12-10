// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "Misc/AutomationTest.h"
#include "SpatialGDKTests/SpatialGDKServices/LocalDeploymentManager/LocalDeploymentManagerUtilities.h"

DEFINE_LOG_CATEGORY_STATIC(LogGDKTestBase, Log, All);

/**
 * This class extends the Unreal AutomationTestBase to allow unit tests to be augmented with a set-up step, called
 * before each test.
 * This class is then offered through a macro, in a similar way to `IMPLEMENT_SIMPLE_AUTOMATION_TEST`.
 *
 * To use this test base, the GDK_AUTOMATION_TEST macro should be used, followed by the test body:
 * ```
 *	   GDK_AUTOMATION_TEST(MyModule, MyComponent, MyTestName)
 *	   {
 *			// do some testing here...
 *
 *			return true;
 *	   }
 * ```
 *
 * Returning `true` indicates a test pass and returning `false` indicates test failure.
 */
class FGDKAutomationTestBase : public FAutomationTestBase
{
public:
	FGDKAutomationTestBase(const FString& Name, bool bInComplexTask, FString TestSrcFileName, uint32 TestSrcFileLine)
		: FAutomationTestBase(Name, bInComplexTask)
		, TestName(Name)
		, TestSourceFileName(TestSrcFileName)
		, TestSourceFileLine(TestSrcFileLine)
	{
	}

	virtual uint32 GetTestFlags() const override
	{
		return EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter;
	}

	virtual bool IsStressTest() { return false; }
	virtual uint32 GetRequiredDeviceNum() const override { return 1; }

protected:
	/**
	 * Checks for an existing deployment and stops it if one exists (also killing the associated workers).
	 * Ran before each test.
	 */
	virtual void SetUp()
	{
		FLocalDeploymentManager* LocalDeploymentManager = SpatialGDK::GetLocalDeploymentManager();
		if (LocalDeploymentManager->IsLocalDeploymentRunning() || LocalDeploymentManager->IsDeploymentStarting())
		{
			UE_LOG(LogGDKTestBase, Warning, TEXT("Deployment found! (Was this left over from another test?)"))
			UE_LOG(LogGDKTestBase, Warning, TEXT("Ending PIE session"))
			GEditor->RequestEndPlayMap();
			ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));
		}
	}

	/**
	 * Implement this method with the test body
	 */
	virtual bool RunGDKTest(const FString& Parameters) = 0;

	virtual bool RunTest(const FString& Parameters) override
	{
		SetUp();
		return RunGDKTest(Parameters);
	}

	virtual FString GetTestSourceFileName() const override { return TestSourceFileName; }

	virtual int32 GetTestSourceFileLine() const override { return TestSourceFileLine; }

	virtual FString GetBeautifiedTestName() const override { return TestName; }

	virtual void GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const override
	{
		OutBeautifiedNames.Add(TestName);
		OutTestCommands.Add(FString());
	}

private:
	FString TestName;
	FString TestSourceFileName;
	uint32 TestSourceFileLine;
};

#define GDK_AUTOMATION_TEST(ModuleName, ComponentName, TestName)                                                                           \
	IMPLEMENT_GDK_AUTOMATION_TEST(TestName, "SpatialGDK." #ModuleName "." #ComponentName "." #TestName)

#define IMPLEMENT_GDK_AUTOMATION_TEST(TestName, PrettyName)                                                                                \
	class TestName : public FGDKAutomationTestBase                                                                                         \
	{                                                                                                                                      \
	public:                                                                                                                                \
		TestName(const FString& InName, bool bInComplexTask, FString TestSourceFileName, uint32 TestSourceFileLine)                        \
			: FGDKAutomationTestBase(InName, bInComplexTask, TestSourceFileName, TestSourceFileLine)                                       \
		{                                                                                                                                  \
		}                                                                                                                                  \
		bool RunGDKTest(const FString& Parameters) override;                                                                               \
	};                                                                                                                                     \
	namespace                                                                                                                              \
	{                                                                                                                                      \
	TestName TestName##__AutomationTestInstance(TEXT(PrettyName), false, __FILE__, __LINE__);                                              \
	}                                                                                                                                      \
	bool TestName::RunGDKTest(const FString& Parameters)
