#pragma once
#include "Misc/AutomationTest.h"
#include "SpatialGDKTests/SpatialGDKServices/LocalDeploymentManager/LocalDeploymentManagerUtilities.h"

DEFINE_LOG_CATEGORY_STATIC(LogGDKTestBase, Log, All);

class GDKTestBase : public FAutomationTestBase
{
public:
	GDKTestBase(const FString& InName, bool bInComplexTask)
		: FAutomationTestBase(InName, bInComplexTask)
	{
	}

protected:
	/**
	 * Before a GDK test runs, it should enforce that it is running upon a clean-slate.
	 * If there is an existing PIE session or deployment, it should be stopped.
	 */
	void SetUp()
	{
		FLocalDeploymentManager* LocalDeploymentManager = GetLocalDeploymentManager();
		if (LocalDeploymentManager->IsLocalDeploymentRunning() || LocalDeploymentManager->IsDeploymentStarting())
		{
			UE_LOG(LogGDKTestBase, Warning, TEXT("Deployment found! (Was this left over from another test?)"))
			UE_LOG(LogGDKTestBase, Warning, TEXT("Ending PIE session"))
			GEditor->RequestEndPlayMap();
			ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));
		}
	}

	/**
	 * Implement this method as the test body
	 */
	virtual bool RunGDKTest(const FString& Params)
	{
		UE_LOG(LogGDKTestBase, Error, TEXT("Running base GDK test"))
		return false;
	}

	bool RunTest(const FString& Params)
	{
		SetUp();
		return RunGDKTest(Params);
	}
};
