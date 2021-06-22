// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestRepNotify.h"

#include "SpatialGDK/Public/EngineClasses/SpatialNetDriver.h"
#include "SpatialTestRepNotifySubobject.h"

#include "Net/UnrealNetwork.h"

/**
 * This test tests RepNotifies and shadow data implementation.
 *
 * The test includes 1 Server and 2 Client workers.
 * The flow is as follows:
 * - Setup:
 *	- The test itself is used as the test Actor.
 *  - It contains a number of replicated properties and OnRep functions that either set boolean flags confirming they have been called, or
 *save the old value of the argument passed to the OnRep function.
 * We also check the ordering of RepNotifies being called, expecting that both an actor and its subobjects will have their new data applied
 *before repnotifies are called on either.
 *
 * - Test:
 *	- The Server changes the values of the replicated properties multiple times.
 *  - The clients check whether the RepNotifies have or have not been called correctly.
 * - Cleanup:
 *	- None.
 */

ASpatialTestRepNotify::ASpatialTestRepNotify()
	: Super()
{
	Author = "Miron + Andrei + Arthur";
	Description = TEXT("Test RepNotify replication and shadow data");

	TestSubobject = CreateDefaultSubobject<USpatialTestRepNotifySubobject>(TEXT("USpatialTestRepNotifySubobject"));
	TestSubobject->SetIsReplicated(true);
}

void ASpatialTestRepNotify::PrepareTest()
{
	Super::PrepareTest();

	// The Server sets some initial values to the replicated variables.
	AddStep(TEXT("SpatialTestRepNotifyServerSetReplicatedVariables"), FWorkerDefinition::Server(1), nullptr, [this]() {
		OnChangedRepNotifyInt1 = 1;
		AlwaysRepNotifyInt1 = 2;
		OnChangedRepNotifyInt2 = 3;
		AlwaysRepNotifyInt2 = 4;
		TestArray.Empty();
		TestArray.Add(1);
		TestArray.Add(2);
		TestSubobject->OnChangedRepNotifyInt = 1;

		FinishStep();
	});

	// All clients check they received the correct values for the replicated variables.
	AddStep(
		TEXT("SpatialTestRepNotifyAllClientsCheckReplicatedVariables"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			if (bOnRepOnChangedRepNotifyInt1Called && bOnRepAlwaysRepNotifyInt1Called && OnChangedRepNotifyInt1 == 1
				&& AlwaysRepNotifyInt1 == 2 && OnChangedRepNotifyInt2 == 3 && AlwaysRepNotifyInt2 == 4 && TestArray.Num() == 2
				&& TestArray[0] == 1 && TestArray[1] == 2)
			{
				FinishStep();
			}
		},
		5.0f);

	// All clients reset the values modified by the RepNotifies called due to the Server modifying the replicated variables.
	AddStep(TEXT("SpatialTestRepNotifiyAllClientsLocallyChangeVariables"), FWorkerDefinition::AllClients, nullptr, [this]() {
		bOnRepOnChangedRepNotifyInt1Called = false;
		bOnRepAlwaysRepNotifyInt1Called = false;
		OldOnChangedRepNotifyInt2 = -3;
		OldAlwaysRepNotifyInt2 = -4;
		OldTestArray.Empty();

		FinishStep();
	});

	// All clients modify 3 of the replicated variables.
	AddStep(
		TEXT("SpatialTestRepNotifyAllClientsModifyReplicatedVariables"), FWorkerDefinition::AllClients, nullptr,
		[this]() {
			// Note that OnChangedRepNotifyInt2 is specifically not modified, this will be relevant in the
			// SpatialTestRepNotifyAllClientsCheckValuesAndRepNotifies step.

			OnChangedRepNotifyInt1 = 10;
			AlwaysRepNotifyInt1 = 20;
			AlwaysRepNotifyInt2 = 50;

			FinishStep();
		},
		nullptr, 5.0f);

	// The Server modifies the replicated variables once again.
	AddStep(TEXT("SpatialTestRepNotifyServerChangeReplicatedVariables"), FWorkerDefinition::Server(1), nullptr, [this]() {
		OnChangedRepNotifyInt1 = 10;
		OnChangedRepNotifyInt2 = 30;

		AlwaysRepNotifyInt1 = 20;
		AlwaysRepNotifyInt2 = 40;

		TestArray.Add(30);
		FinishStep();
	});

	// All clients check that the replicated variables were received correctly and that RepNotify acted as expected.
	AddStep(
		TEXT("SpatialTestRepNotifyAllClientsCheckValuesAndRepNotifies"), FWorkerDefinition::AllClients,
		[this]() -> bool {
			// First make sure that we have correctly received the replicated variables, before checking the RepNotify behaviour.
			if (!(OnChangedRepNotifyInt1 == 10 && AlwaysRepNotifyInt1 == 20 && OnChangedRepNotifyInt2 == 30 && AlwaysRepNotifyInt2 == 40))
			{
				return false;
			}

			if (!(TestArray.Num() == 3 && TestArray[0] == 1 && TestArray[1] == 2 && TestArray[2] == 30))
			{
				return false;
			}

			return true;
		},
		[this]() {
			// At this point, we have correctly received the values of all replicated variables, therefore the RepNotify behaviour can be
			// checked.

			// Since the RepNotify for this variable is using the default REPNOTIFY_OnChanged, we expect it not to get called on the
			// clients.
			if (bOnRepOnChangedRepNotifyInt1Called)
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepOnChangedRepNotifyInt1 should not be called on the clients"));
				return;
			}

			// In this case, the RepNotify uses REPNOTIFY_Always so we expect it to be called on the clients.
			if (!bOnRepAlwaysRepNotifyInt1Called)
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepOnChangedRepNotifyInt1 should be called on the clients"));
				return;
			}

			// From the Clients, we have not modified the value of this particular variable, so we still expect the old value to be the one
			// initially set by the Server in the first step.
			if (OldOnChangedRepNotifyInt2 != 3)
			{
				FinishTest(EFunctionalTestResult::Failed,
						   TEXT("OnRepOnChangedRepNotifyInt2 should have been called with the old value of 3"));
				return;
			}

			// Since we have modified this value from the Clients, we expect its value to be the same as set in
			// SpatialTestRepNotifyAllClientsModifyReplicatedVariables.
			if (OldAlwaysRepNotifyInt2 != 50)
			{
				FinishTest(EFunctionalTestResult::Failed,
						   TEXT("OnRepAlwaysRepNotifyInt2 should have been called with the old value of 40"));
				return;
			}

		// We consciously differ from native UE here
		// Also, the native behaviour changed when going from 4.25 to 4.26.
		// On older versions, we expect the old array to have 3 elements, but on Spatial and on native starting from 4.26, we expect 2
		// elements.
#if ENGINE_MINOR_VERSION >= 26
			bool bOldArrayShouldHaveTwoElements = true;
#else
			bool bOldArrayShouldHaveTwoElements = false;
#endif
			bOldArrayShouldHaveTwoElements = bOldArrayShouldHaveTwoElements || GetNetDriver()->IsA(USpatialNetDriver::StaticClass());
			if (bOldArrayShouldHaveTwoElements)
			{
				if (OldTestArray.Num() != 2)
				{
					FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestArray should have been called with 2 entries in the old Array "
																   "on Spatial or in Native on 4.26 and above"));
					return;
				}
			}
			else
			{
				if (OldTestArray.Num() != 3)
				{
					FinishTest(EFunctionalTestResult::Failed,
							   TEXT("OnRepTestArray should have been called with 3 entries in the old Array on Native in 4.25 and below"));
					return;
				}

				if (OldTestArray[2] != 0)
				{
					FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestArray should have been called with 0 as its third entry in "
																   "the old Array on Native in 4.25 and below"));
					return;
				}
			}

			if (OldTestArray[0] != 1)
			{
				FinishTest(EFunctionalTestResult::Failed,
						   TEXT("OnRepTestArray should have been called with 1 as its first entry in the old Array"));
				return;
			}

			if (OldTestArray[1] != 2)
			{
				FinishTest(EFunctionalTestResult::Failed,
						   TEXT("OnRepTestArray should have been called with 2 as its second entry in the old Array"));
				return;
			}

			FinishStep();
		},
		nullptr, 5.0f);

	AddStep(TEXT("SpatialTestRepNotifyServerTestRemovalFromArray"), FWorkerDefinition::Server(1), nullptr, [this]() {
		TestArray.Pop(true);

		FinishStep();
	});

	AddStep(
		TEXT("SpatialTestRepNotifyClientsCheckArrayRemoval"), FWorkerDefinition::AllClients,
		[this]() -> bool {
			// First make sure that we have correctly received the replicated variables, before checking RepNotify behaviour.
			return TestArray.Num() == 2;
		},
		[this]() {
	// At this point, we have received the update for the TestArray, so it makes sense to check RepNotify beahviour.

	// We consciously differ from native UE here
	// Also, the native behaviour changed when going from 4.25 to 4.26.
	// On older versions, we expect the old array to have 2 elements, but on Spatial and on native starting from 4.26, we expect 3 elements.
#if ENGINE_MINOR_VERSION >= 26
			bool bOldArrayShouldHaveThreeElements = true;
#else
			bool bOldArrayShouldHaveThreeElements = false;
#endif
			bOldArrayShouldHaveThreeElements = bOldArrayShouldHaveThreeElements || GetNetDriver()->IsA(USpatialNetDriver::StaticClass());
			if (bOldArrayShouldHaveThreeElements)
			{
				if (OldTestArray.Num() != 3)
				{
					FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestArray should have been called with 3 elements after shrinking "
																   "on Spatial or in Native on 4.26 and above"));
					return;
				}
				if (OldTestArray[2] != 30)
				{
					FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestArray should have been called with 30 as its third entry "
																   "after shrinking on Spatial or in Native on 4.26 and above"));
					return;
				}
			}
			else
			{
				if (OldTestArray.Num() != 2)
				{
					FinishTest(EFunctionalTestResult::Failed,
							   TEXT("OnRepTestArray should have been called with 2 elements after shrinking on Native on 4.25 and below"));
					return;
				}
			}

			FinishStep();
		},
		nullptr, 5.0f);

	/**
	 * In this section of the test, we check the order rep notifies are called in.
	 * In native, data will be applied for an actor, its subobjects, and only THEN will repnotifies be called for the updated data.
	 *
	 * To test this, we set the update a replicated property on an actor (the test itself) and a subobject.
	 * Then in repnotifies on both the actor and the subobject, we check that the property on the other object has already been updated.
	 * */

	AddStep(
		TEXT("SpatialTestRepNotifyAllWorkersInitialiseExpectedOrderingProps"), FWorkerDefinition::AllWorkers, nullptr,
		[this]() {
			AssertIsValid(TestSubobject, TEXT("TestSubobject should be valid."));
			TestSubobject->ExpectedParentInt1Property = 350;
			TestSubobject->bParentPropertyWasExpectedProperty = false;
			ExpectedSubobjectIntProperty = 400;
			bSubobjectIntPropertyWasExpectedProperty = false;
			FinishStep();
		},
		nullptr);

	AddStep(
		TEXT("SpatialTestRepNotifyServerChangeReplicatedVariables2"), FWorkerDefinition::Server(1), nullptr,
		[this]() {
			AssertIsValid(TestSubobject, TEXT("TestSubobject should be valid."));
			TestSubobject->OnChangedRepNotifyInt = 400;
			OnChangedRepNotifyInt1 = 350;
			FinishStep();
		},
		nullptr);

	AddStep(
		TEXT("SpatialTestRepNotifyClientCheckOrderingWasCorrect"), FWorkerDefinition::AllClients,
		[this]() -> bool {
			RequireEqual_Int(TestSubobject->OnChangedRepNotifyInt, 400,
							 TEXT("The subobject's OnChangedRepNotifyInt property should have been updated to 400"));
			RequireEqual_Int(OnChangedRepNotifyInt1, 350,
							 TEXT("The actors OnChangedRepNotifyInt1 property should have been updated to 350"));
			return true;
		},
		[this]() {
			AssertIsValid(TestSubobject, TEXT("TestSubobject should be valid."));
			AssertEqual_Bool(TestSubobject->bParentPropertyWasExpectedProperty, true,
							 TEXT("The OnChangedRepNotifyInt1 on parent actor ASpatialTestRepNotify should have been set to 350 before the "
								  "subobject's RepNotify was called"));
			AssertEqual_Bool(bSubobjectIntPropertyWasExpectedProperty, true,
							 TEXT("The OnChangedRepNotifyInt1 on subobject USpatialTestRepNotifySubobject should have been set to 400 "
								  "before the actor's RepNotify was called"));
			FinishStep();
		},
		nullptr);
}

void ASpatialTestRepNotify::OnRep_OnChangedRepNotifyInt1(int32 OldOnChangedRepNotifyInt1)
{
	bOnRepOnChangedRepNotifyInt1Called = true;
	bSubobjectIntPropertyWasExpectedProperty = false;

	ensureAlwaysMsgf(IsValid(TestSubobject), TEXT("TestSubobject should be valid"));

	bSubobjectIntPropertyWasExpectedProperty = TestSubobject->OnChangedRepNotifyInt == ExpectedSubobjectIntProperty;
}

void ASpatialTestRepNotify::OnRep_AlwaysRepNotifyInt1(int32 OldAlwaysRepNotifyInt1)
{
	bOnRepAlwaysRepNotifyInt1Called = true;
}

void ASpatialTestRepNotify::OnRep_OnChangedRepNotifyInt2(int32 InOldOnChangedRepNotifyInt2)
{
	OldOnChangedRepNotifyInt2 = InOldOnChangedRepNotifyInt2;
}

void ASpatialTestRepNotify::OnRep_AlwaysRepNotifyInt2(int32 InOldAlwaysRepNotifyInt2)
{
	OldAlwaysRepNotifyInt2 = InOldAlwaysRepNotifyInt2;
}

void ASpatialTestRepNotify::OnRep_TestArray(TArray<int32> InOldTestArray)
{
	OldTestArray = InOldTestArray;
}

void ASpatialTestRepNotify::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTestRepNotify, OnChangedRepNotifyInt1);
	DOREPLIFETIME_CONDITION_NOTIFY(ASpatialTestRepNotify, AlwaysRepNotifyInt1, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(ASpatialTestRepNotify, OnChangedRepNotifyInt2);
	DOREPLIFETIME_CONDITION_NOTIFY(ASpatialTestRepNotify, AlwaysRepNotifyInt2, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(ASpatialTestRepNotify, TestArray);

	DOREPLIFETIME(ASpatialTestRepNotify, TestSubobject);
}
