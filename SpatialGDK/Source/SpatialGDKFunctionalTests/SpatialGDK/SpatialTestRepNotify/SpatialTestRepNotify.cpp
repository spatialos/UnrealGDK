// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestRepNotify.h"

#include "SpatialTestRepNotifyActor.h"
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
}

void ASpatialTestRepNotify::PrepareTest()
{
	Super::PrepareTest();

	/**
	* Here we check core rep notify functionality and the order rep notifies are called in.
	* In native, data will be applied for an actor, its subobjects, and only THEN will repnotifies be called for the updated data.
	*
	* To test this, we set the update a replicated property on an actor and a subobject of its.
	* Then in repnotifies on both the actor and the subobject, we check that the property on the other object has already been updated.
	* */
	AddStep(TEXT("SpatialTestRepNotifyServerSetReplicatedVariables1"), FWorkerDefinition::Server(1), nullptr, [this]() {
		TestActor = GetWorld()->SpawnActor<ASpatialTestRepNotifyActor>(FVector(0, 0, 0), FRotator::ZeroRotator, FActorSpawnParameters());
		RegisterAutoDestroyActor(TestActor);

		TestActor->TestSubobject->OnChangedRepNotifyInt = 400;
		TestActor->OnChangedRepNotifyInt1 = 350;

		FinishStep();
	});

	AddStep(
		TEXT("SpatialTestRepNotifyClientCheckOrderingWasCorrect"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			if (!RequireEqual_Bool(IsValid(TestActor), true, TEXT("TestActor should be valid.")))
			{
				return;
			}
			if (!RequireEqual_Bool(IsValid(TestActor->TestSubobject), true, TEXT("TestActor's subobject should be valid.")))
			{
				return;
			}


			RequireEqual_Int(TestActor->TestSubobject->OnChangedRepNotifyInt, 400,
							 TEXT("The subobject's OnChangedRepNotifyInt property should have been updated to 400."));
			RequireEqual_Int(TestActor->OnChangedRepNotifyInt1, 350,
							 TEXT("The actors OnChangedRepNotifyInt1 property should have been updated to 350."));

			RequireEqual_Bool(
				TestActor->TestSubobject->bParentPropertyWasExpectedProperty, true,
				TEXT("The OnChangedRepNotifyInt1 on parent actor ASpatialTestRepNotifyActor should have been set to 350 before the "
					 "subobject's RepNotify was called."));
			RequireEqual_Bool(TestActor->bSubobjectIntPropertyWasExpectedProperty, true,
							  TEXT("The OnChangedRepNotifyInt1 on subobject USpatialTestRepNotifySubobject should have been set to 400 "
								   "before the actor's RepNotify was called."));
			FinishStep();
		},
		5.0f);

	// Further core Rep Notify functionality testing
	// The Server modifies the replicated variables.
	AddStep(TEXT("SpatialTestRepNotifyServerChangeReplicatedVariables2"), FWorkerDefinition::Server(1), nullptr, [this]()
	{
		TestActor->OnChangedRepNotifyInt1 = 1;
		TestActor->AlwaysRepNotifyInt1 = 2;
		TestActor->OnChangedRepNotifyInt2 = 3;
		TestActor->AlwaysRepNotifyInt2 = 4;
		TestActor->TestArray.Empty();
		TestActor->TestArray.Add(1);
		TestActor->TestArray.Add(2);
		TestActor->TestSubobject->OnChangedRepNotifyInt = 2;
		FinishStep();
	});

	// All clients check they received the correct values for the replicated variables.
	AddStep(
		TEXT("SpatialTestRepNotifyAllClientsCheckReplicatedVariables"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireEqual_Bool(TestActor->bOnRepOnChangedRepNotifyInt1Called, true, TEXT("Test actor's OnChangedInt1 Rep Notify should have been called"));
			RequireEqual_Bool(TestActor->bOnRepAlwaysRepNotifyInt1Called, true, TEXT("Test actor's AlwaysInt1 Rep Notify should have been called"));

			RequireEqual_Int(TestActor->OnChangedRepNotifyInt1, 1, TEXT("Test actor's OnChangedRepNotifyInt1 property should be 1"));
			RequireEqual_Int(TestActor->AlwaysRepNotifyInt1, 2, TEXT("Test actor's AlwaysRepNotifyInt1 property should be 2"));
			RequireEqual_Int(TestActor->OnChangedRepNotifyInt2, 3, TEXT("Test actor's OnChangedRepNotifyInt2 property should be 3"));
			RequireEqual_Int(TestActor->AlwaysRepNotifyInt2, 4, TEXT("Test actor's AlwaysRepNotifyInt2 property should be 4"));
			if (RequireEqual_Int(TestActor->TestArray.Num(), 2, TEXT("Test actor's TestArray should have 2 elements")))
			{
				RequireEqual_Int(TestActor->TestArray[0], 1, TEXT("Test actor's TestArray should have 1 at index 0"));
				RequireEqual_Int(TestActor->TestArray[1], 2, TEXT("Test actor's TestArray should have 2 at index 1"));
			}

			RequireEqual_Int(TestActor->TestSubobject->OnChangedRepNotifyInt, 2, TEXT("Test actor subobject's OnChangedRepNotifyInt property should be 2"));

			FinishStep();
		},
		5.0f);

	// All clients reset the values modified by the RepNotifies called due to the Server modifying the replicated variables.
	AddStep(TEXT("SpatialTestRepNotifiyAllClientsLocallyChangeVariables"), FWorkerDefinition::AllClients, nullptr, [this]() {
		TestActor->bOnRepOnChangedRepNotifyInt1Called = false;
		TestActor->bOnRepAlwaysRepNotifyInt1Called = false;
		TestActor->OldOnChangedRepNotifyInt2 = -3;
		TestActor->OldAlwaysRepNotifyInt2 = -4;
		TestActor->OldTestArray.Empty();

		FinishStep();
	});

	// All clients modify 3 of the replicated variables.
	AddStep(
		TEXT("SpatialTestRepNotifyAllClientsModifyReplicatedVariables"), FWorkerDefinition::AllClients, nullptr,
		[this]() {
			// Note that OnChangedRepNotifyInt2 is specifically not modified, this will be relevant in the
			// SpatialTestRepNotifyAllClientsCheckValuesAndRepNotifies step.

			TestActor->OnChangedRepNotifyInt1 = 10;
			TestActor->AlwaysRepNotifyInt1 = 20;
			TestActor->AlwaysRepNotifyInt2 = 50;

			FinishStep();
		},
		nullptr, 5.0f);

	// The Server modifies the replicated variables once again.
	AddStep(TEXT("SpatialTestRepNotifyServerChangeReplicatedVariables3"), FWorkerDefinition::Server(1), nullptr, [this]() {
		TestActor->OnChangedRepNotifyInt1 = 10;
		TestActor->OnChangedRepNotifyInt2 = 30;

		TestActor->AlwaysRepNotifyInt1 = 20;
		TestActor->AlwaysRepNotifyInt2 = 40;

		TestActor->TestArray.Add(30);
		FinishStep();
	});

	// All clients check that the replicated variables were received correctly and that RepNotify acted as expected.
	AddStep(
		TEXT("SpatialTestRepNotifyAllClientsCheckValuesAndRepNotifies"), FWorkerDefinition::AllClients,
		nullptr, nullptr,
		[this](float DeltaTime) {

			// First make sure that we have correctly received the replicated variables.
			RequireEqual_Int(TestActor->OnChangedRepNotifyInt1, 10, TEXT("Test actor's OnChangedRepNotifyInt1 property should be 10"));
			RequireEqual_Int(TestActor->AlwaysRepNotifyInt1, 20, TEXT("Test actor's AlwaysRepNotifyInt1 property should be 20"));
			RequireEqual_Int(TestActor->OnChangedRepNotifyInt2, 30, TEXT("Test actor's OnChangedRepNotifyInt2 property should be 30"));
			RequireEqual_Int(TestActor->AlwaysRepNotifyInt2, 40, TEXT("Test actor's AlwaysRepNotifyInt2 property should be 40"));

			if (RequireEqual_Int(TestActor->TestArray.Num(), 3, TEXT("Test actor's TestArray should have 3 elements")))
			{
				RequireEqual_Int(TestActor->TestArray[0], 1, TEXT("Test actor's TestArray should have 1 at index 0"));
				RequireEqual_Int(TestActor->TestArray[1], 2, TEXT("Test actor's TestArray should have 2 at index 1"));
				RequireEqual_Int(TestActor->TestArray[2], 30, TEXT("Test actor's TestArray should have 30 at index 2"));
			}


			// At this point, we have correctly received the values of all replicated variables, therefore the RepNotify behaviour can be
			// checked.

			// Since the RepNotify for this variable is using the default REPNOTIFY_OnChanged, we expect it not to get called on the
			// clients.
			RequireEqual_Bool(TestActor->bOnRepOnChangedRepNotifyInt1Called, false, TEXT("Test actor's OnChangedInt1 Rep Notify should not be called as the value stayed the same"));
			RequireEqual_Bool(TestActor->bOnRepAlwaysRepNotifyInt1Called, true, TEXT("Test actor's AlwaysInt1 Rep Notify should be called"));


			// From the Clients, we have not modified the value of this particular variable, so we still expect the old value to be the one
			// initially set by the Server in the first step.
			RequireEqual_Int(TestActor->OldOnChangedRepNotifyInt2, 3, TEXT("OnRepOnChangedRepNotifyInt2 should be called with the old value of 3"));

			// Since we have modified this value from the Clients, we expect its value to be the same as set in
			// SpatialTestRepNotifyAllClientsModifyReplicatedVariables.
			RequireEqual_Int(TestActor->OldAlwaysRepNotifyInt2, 50, TEXT("OnRepAlwaysRepNotifyInt2 should be called with the old value of 40"));

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
				RequireEqual_Int(TestActor->OldTestArray.Num(), 2, TEXT("OnRepTestArray should been called with 2 entries in the old Array "
															"on Spatial or in Native on 4.26 and above"));
			}
			else
			{
				if (RequireEqual_Int(TestActor->OldTestArray.Num(), 3, TEXT("OnRepTestArray should be called with 3 entries in the old Array on Native in 4.25 and below")))
				{
					RequireEqual_Int(TestActor->OldTestArray[2], 0, TEXT("OnRepTestArray should be called with 0 as its third entry in "
												"the old Array on Native in 4.25 and below"));
				}
			}

			if (TestActor->OldTestArray.Num() >= 2)
			{
				RequireEqual_Int(TestActor->OldTestArray[0], 1, TEXT("OnRepTestArray should be called with 1 as its first entry in the old Array"));
				RequireEqual_Int(TestActor->OldTestArray[1], 2, TEXT("OnRepTestArray should be called with 2 as its second entry in the old Array"));
			}

			FinishStep();
		},
		5.0f);

	AddStep(TEXT("SpatialTestRepNotifyServerTestRemovalFromArray"), FWorkerDefinition::Server(1), nullptr, [this]() {
		TestActor->TestArray.Pop(true);

		FinishStep();
	});

	AddStep(
		TEXT("SpatialTestRepNotifyClientsCheckArrayRemoval"), FWorkerDefinition::AllClients,
		nullptr, nullptr,
		[this](float DeltaTime) {
			// First make sure that we have correctly received the replicated variables
			RequireEqual_Int(TestActor->TestArray.Num(), 2, TEXT("TestArray should contain 2 elements."));

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
				if (RequireEqual_Int(TestActor->OldTestArray.Num(), 3, TEXT("OnRepTestArray should be called with 3 elements after shrinking "
																"on Spatial or in Native on 4.26 and above")))
				{
					RequireEqual_Int(TestActor->OldTestArray[2], 30, TEXT("OnRepTestArray should be called with 30 as its third entry "
												"after shrinking on Spatial or in Native on 4.26 and above"));
				}
			}
			else
			{
				RequireEqual_Int(TestActor->OldTestArray.Num(), 2, TEXT("OnRepTestArray should be called with 2 elements after shrinking on Native on 4.25 and below"));
			}

			FinishStep();
		},
		5.0f);
}

void ASpatialTestRepNotify::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTestRepNotify, TestActor);
}
