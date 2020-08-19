// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestRepNotify.h"
#include "SpatialGDK/Public/EngineClasses/SpatialNetDriver.h"

#include "Net/UnrealNetwork.h"

/**
 * This test tests RepNotifies and shadow data implementation.
 *
 * The test includes 1 Server and 2 Client workers.
 * The flow is as follows:
 * - Setup:
 *	- The test itself is used as the test Actor.
 *  - It contains a number of replicated properties and OnRep functions that either set boolean flags confirming they have been called, or save the old value of the argument passed to the OnRep function.
 * - Test:
 *	- The Server changes the values of the replicated properties multiple times.
 *  - The clients check whether the RepNotifies have or have not been called correctly.
 * - Cleanup:
 *	- None.
 */

ASpatialTestRepNotify::ASpatialTestRepNotify()
	: Super()
{
	Author = "Miron";
	Description = TEXT("Test RepNotify replication and shadow data");
}

void ASpatialTestRepNotify::BeginPlay()
{
	Super::BeginPlay();

	// The Server sets some initial values to the replicated variables.
	AddStep(TEXT("SpatialTestRepNotifyServerSetReplicatedVariables"), FWorkerDefinition::Server(1), nullptr, [this]()
		{
			TestInt1 = 1;
			TestInt2 = 2;
			TestInt3 = 3;
			TestInt4 = 4;
			TestArray.Empty();
			TestArray.Add(1);
			TestArray.Add(2);

			FinishStep();
		});

	// All clients check they received the correct values for the replicated variables.
	AddStep(TEXT("SpatialTestRepNotifyAllClientsCheckReplicatedVariables"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](float DeltaTime)
		{
			if (bOnRepTestInt1Called && bOnRepTestInt2Called
				&& TestInt1 == 1 && TestInt2 == 2 && TestInt3 == 3 && TestInt4 == 4
				&& TestArray.Num() == 2 && TestArray[0] == 1 && TestArray[1] == 2)
			{
				FinishStep();
			}
		}, 5.0f);

	// All clients reset the values modified by the RepNotifies called due to the Server modifying the replicated variables.
	AddStep(TEXT("SpatialTestRepNotifiyAllClientsLocallyChangeVariables"), FWorkerDefinition::AllClients, nullptr, [this]()
		{
			bOnRepTestInt1Called = false;
			bOnRepTestInt2Called = false;
			OldTestInt3 = -3;
			OldTestInt4 = -4;
			OldTestArray.Empty();

			FinishStep();
		});

	// All clients modify 3 of the replicated variables.
	AddStep(TEXT("SpatialTestRepNotifyAllClientsModifyReplicatedVariables"), FWorkerDefinition::AllClients, [this]() -> bool
		{
			// Extra check to ensure that the Old values are correct before changing the replicated variables.
			if (OldTestInt3 == -3 && OldTestInt4 == -4)
			{
				return true;
			}

			return false;
		}, [this]()
		{
			// Note that TestInt3 is specifically not modified, this will be relevant in the SpatialTestRepNotifyAllClientsCheckValuesAndRepNotifies step.

			TestInt1 = 10;
			TestInt2 = 20;
			TestInt4 = 50;

			FinishStep();
		}, nullptr, 5.0f);

	// The Server modifies the replicated variables once again.
	AddStep(TEXT("SpatialTestRepNotifyServerChangeReplicatedVariables"), FWorkerDefinition::Server(1), nullptr, [this]()
		{
			TestInt1 = 10;
			TestInt2 = 20;
			TestInt3 = 30;
			TestInt4 = 40;

			TestArray.Add(30);
			FinishStep();
		});

	// All clients check that the replicated variables were received correctly and that RepNotify acted as expected.
	AddStep(TEXT("SpatialTestRepNotifyAllClientsCheckValuesAndRepNotifies"), FWorkerDefinition::AllClients, [this]()->bool
		{
			// First make sure that we have correctly received the replicated variables, before checking the RepNotify behaviour.
			if (!(TestInt1 == 10 && TestInt2 == 20 && TestInt3 == 30 && TestInt4 == 40))
			{
				return false;
			}

			if (!(TestArray.Num() == 3 && TestArray[0] == 1 && TestArray[1] == 2 && TestArray[2] == 30))
			{
				return false;
			}

			return true;
		}, [this]()
		{
			// At this point, we have correctly received the values of all replicated variables, therefore the RepNotify behaviour can be checked.

			// Since the RepNotify for this variable is using the default REPNOTIFY_OnChanged, we expect it not to get called on the clients.
			if (bOnRepTestInt1Called)
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestInt1 should not be called on the clients"));
				return;
			}

			// In this case, the RepNotify uses REPNOTIFY_Always so we expect it to be called on the clients.
			if (!bOnRepTestInt2Called)
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestInt1 should be called on the clients"));
				return;
			}

			// From the Clients, we have not modified the value of this particular variable, so we still expect the old value to be the one initially set by the Server in the first step.
			if (OldTestInt3 != 3)
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestInt3 should have been called with the old value of 3"));
				return;
			}

			// Since we have modified this value from the Clients, we expect its value to be the same as set in SpatialTestRepNotifyAllClientsModifyReplicatedVariables.
			if (OldTestInt4 != 50)
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestInt4 should have been called with the old value of 40"));
				return;
			}

			// We consciously differ from native UE here
			if (GetNetDriver()->IsA(USpatialNetDriver::StaticClass()))
			{
				if (OldTestArray.Num() != 2)
				{
					FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestArray should have been called with 2 entries in the old Array on Spatial"));
					return;
				}
			}
			else
			{
				if (OldTestArray.Num() != 3)
				{
					FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestArray should have been called with 3 entries in the old Array on Native"));
					return;
				}

				if (OldTestArray[2] != 0)
				{
					FinishTest(EFunctionalTestResult::Failed,  TEXT("OnRepTestArray should have been called with 0 as its third entry in the old Array on Native"));
					return;
				}
			}

			if (OldTestArray[0] != 1)
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestArray should have been called with 1 as its first entry in the old Array"));
				return;
			}

			if (OldTestArray[1] != 2)
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestArray should have been called with 2 as its second entry in the old Array"));
				return;
			}

			FinishStep();
		}, nullptr, 5.0f);

	AddStep(TEXT("SpatialTestRepNotifyServerTestRemovalFromArray"), FWorkerDefinition::Server(1), nullptr, [this]()
		{
			TestArray.Pop(true);

			FinishStep();
		});

	AddStep(TEXT("SpatialTestRepNotifyClientsCheckArrayRemoval"), FWorkerDefinition::AllClients, [this]() -> bool
		{
			// First make sure that we have correctly received the replicated variables, before checking RepNotify behaviour.
			return TestArray.Num() == 2;
		}, [this]()
		{
			// At this point, we have received the update for the TestArray, so it makes sense to check RepNotify beahviour.

			// We consciously differ from native UE here
			if (GetNetDriver()->IsA(USpatialNetDriver::StaticClass()))
			{
				if (OldTestArray.Num() != 3)
				{
					FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestArray should have been called with 3 elements after shrinking on Spatial"));
					return;
				}
				if (OldTestArray[2] != 30)
				{
					FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestArray should have been called with 30 as its third entry after shrinking on Spatial"));
					return;
				}
			}
			else
			{
				if (OldTestArray.Num() != 2)
				{
					FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestArray should have been called with 2 elements after shrinking on Native"));
					return;
				}
			}

			FinishStep();
		}, nullptr, 5.0f);
}

void ASpatialTestRepNotify::OnRep_TestInt1(int32 OldTestInt1)
{
	bOnRepTestInt1Called = true;
}

void ASpatialTestRepNotify::OnRep_TestInt2(int32 OldTestInt2)
{
	bOnRepTestInt2Called = true;
}

void ASpatialTestRepNotify::OnRep_TestInt3(int32 InOldTestInt3)
{
	OldTestInt3 = InOldTestInt3;
}

void ASpatialTestRepNotify::OnRep_TestInt4(int32 InOldTestInt4)
{
	OldTestInt4 = InOldTestInt4;
}

void ASpatialTestRepNotify::OnRep_TestArray(TArray<int32> InOldTestArray)
{
	OldTestArray = InOldTestArray;
}

void ASpatialTestRepNotify::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTestRepNotify, TestInt1);
	DOREPLIFETIME_CONDITION_NOTIFY(ASpatialTestRepNotify, TestInt2, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(ASpatialTestRepNotify, TestInt3);
	DOREPLIFETIME_CONDITION_NOTIFY(ASpatialTestRepNotify, TestInt4, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(ASpatialTestRepNotify, TestArray);
}
