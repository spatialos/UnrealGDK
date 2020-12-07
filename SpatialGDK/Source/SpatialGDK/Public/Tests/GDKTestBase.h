// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "Misc/AutomationTest.h"
// #include "SpatialGDKTests/SpatialGDKServices/LocalDeploymentManager/LocalDeploymentManagerUtilities.h"

DEFINE_LOG_CATEGORY_STATIC(LogGDKTestBase, Log, All);

class FGDKTestBase : public FAutomationTestBase
{
public:
	FGDKTestBase(const FString& InName, bool bInComplexTask)
		: FAutomationTestBase(InName, bInComplexTask)
	{
	}
};

#define IMPLEMENT_GDK_AUTOMATION_TEST(TestName, PrettyName)                                                                                \
	class TestName : public FGDKTestBase                                                                                                   \
	{                                                                                                                                      \
	public:                                                                                                                                \
		TestName(const FString& InName)                                                                                                    \
			: FGDKTestBase(InName, false)                                                                                                  \
		{                                                                                                                                  \
		}                                                                                                                                  \
		virtual uint32 GetTestFlags() const override                                                                                       \
		{                                                                                                                                  \
			return EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter;                                      \
		}                                                                                                                                  \
		virtual bool IsStressTest() const { return false; }                                                                                \
		virtual uint32 GetRequiredDeviceNum() const override { return 1; }                                                                 \
		virtual FString GetTestSourceFileName() const override { return __FILE__; }                                                        \
		virtual int32 GetTestSourceFileLine() const override { return __LINE__; }                                                          \
                                                                                                                                           \
	protected:                                                                                                                             \
		void SetUp()                                                                                                                       \
		{                                                                                                                                  \
			FLocalDeploymentManager* LocalDeploymentManager = GetLocalDeploymentManager();                                                 \
			if (LocalDeploymentManager->IsLocalDeploymentRunning() || LocalDeploymentManager->IsDeploymentStarting())                      \
			{                                                                                                                              \
				UE_LOG(LogGDKTestBase, Warning, TEXT("Deployment found! (Was this left over from another test?)"))                         \
				UE_LOG(LogGDKTestBase, Warning, TEXT("Ending PIE session"))                                                                \
				GEditor->RequestEndPlayMap();                                                                                              \
				ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));                                   \
			}                                                                                                                              \
		}                                                                                                                                  \
		virtual void GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const override                        \
		{                                                                                                                                  \
			OutBeautifiedNames.Add(PrettyName);                                                                                            \
			OutTestCommands.Add(FString());                                                                                                \
		}                                                                                                                                  \
		virtual bool RunTest(const FString& Parameters) override                                                                           \
		{                                                                                                                                  \
			SetUp();                                                                                                                       \
			return RunGDKTest(Parameters);                                                                                                 \
		}                                                                                                                                  \
		virtual bool RunGDKTest(const FString& Parameters);                                                                                \
		virtual FString GetBeautifiedTestName() const override { return TEXT(PrettyName); }                                                \
	};                                                                                                                                     \
	namespace                                                                                                                              \
	{                                                                                                                                      \
	TestName TestName##AutomationTestInstance(TEXT(PrettyName));                                                                           \
	}                                                                                                                                      \
	bool TestName::RunGDKTest(const FString& Parameters)
