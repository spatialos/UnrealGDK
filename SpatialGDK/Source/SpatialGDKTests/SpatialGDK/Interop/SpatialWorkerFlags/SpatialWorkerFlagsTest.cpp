// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialWorkerFlags.h"

#include "SpatialGDKTests/Public/GDKAutomationTestBase.h"
#include "Tests/TestDefinitions.h"
#include "WorkerFlagsTestSpyObject.h"

#define SPATIALWORKERFLAGS_TEST(TestName) GDK_AUTOMATION_TEST(Core, SpatialWorkerFlags, TestName)

namespace
{
const FString TestWorkerFlagKey = TEXT("test");
const FString TestWorkerFlagKey2 = TEXT("test2");
const FString TestWorkerFlagValue = TEXT("10");
const FString TestWorkerFlagValue2 = TEXT("20");
} // anonymous namespace

SPATIALWORKERFLAGS_TEST(GIVEN_a_flagUpdate_op_WHEN_adding_a_worker_flag_THEN_flag_added)
{
	// GIVEN
	// Add test flag
	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	// WHEN
	SpatialWorkerFlags->SetWorkerFlag(TestWorkerFlagKey, TestWorkerFlagValue);
	// THEN
	FString OutFlagValue;
	TestTrue("Flag added in the WorkerFlags map: ", SpatialWorkerFlags->GetWorkerFlag(TestWorkerFlagKey, OutFlagValue));
	TestEqual("Correct value stored in the WorkerFlags map: ", OutFlagValue, TestWorkerFlagValue);
	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_a_flagUpdate_op_WHEN_removing_a_worker_flag_THEN_flag_removed)
{
	// GIVEN
	// Add test flag
	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpatialWorkerFlags->SetWorkerFlag(TestWorkerFlagKey, TestWorkerFlagValue);
	FString OutFlagValue;
	TestTrue("Flag added in the WorkerFlags map: ", SpatialWorkerFlags->GetWorkerFlag(TestWorkerFlagKey, OutFlagValue));
	TestEqual("Correct value stored in the WorkerFlags map: ", OutFlagValue, TestWorkerFlagValue);
	// WHEN
	// Remove test flag
	SpatialWorkerFlags->RemoveWorkerFlag(TestWorkerFlagKey);
	// THEN
	TestFalse("Flag removed from the WorkerFlags map: ", SpatialWorkerFlags->GetWorkerFlag(TestWorkerFlagKey, OutFlagValue));
	return true;
}

// Delegates
// Any flag updates
SPATIALWORKERFLAGS_TEST(GIVEN_a_registered_any_flag_update_delegate_WHEN_any_worker_flag_updates_THEN_delegate_invoked)
{
	// GIVEN
	// Register callback
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnAnyWorkerFlagUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetAnyFlagUpdated);
	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpatialWorkerFlags->RegisterAnyFlagUpdatedCallback(WorkerFlagDelegate);
	// WHEN
	// Update flag
	SpatialWorkerFlags->SetWorkerFlag(TestWorkerFlagKey, TestWorkerFlagValue);
	// Update another flag
	SpatialWorkerFlags->SetWorkerFlag(TestWorkerFlagKey2, TestWorkerFlagValue2);
	// THEN
	TestEqual("Delegate Function was called twice", SpyObj->GetTimesFlagUpdated(), 2);
	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_a_registered_any_flag_update_delegate_WHEN_unregistering_the_delegate_THEN_delegate_is_not_invoked)
{
	// GIVEN
	// Register callback
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnAnyWorkerFlagUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetAnyFlagUpdated);
	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpatialWorkerFlags->RegisterAnyFlagUpdatedCallback(WorkerFlagDelegate);
	// Add test flag
	SpatialWorkerFlags->SetWorkerFlag(TestWorkerFlagKey, TestWorkerFlagValue);
	TestEqual("Delegate Function was called", SpyObj->GetTimesFlagUpdated(), 1);
	// WHEN
	// Unregister callback
	SpatialWorkerFlags->UnregisterAnyFlagUpdatedCallback(WorkerFlagDelegate);
	// Update test flag again
	SpatialWorkerFlags->SetWorkerFlag(TestWorkerFlagKey, TestWorkerFlagValue);
	// THEN
	TestEqual("Delegate Function was called only once", SpyObj->GetTimesFlagUpdated(), 1);
	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_an_updated_flag_WHEN_registering_and_invoking_an_any_flag_update_delegate_THEN_delegate_is_invoked)
{
	// GIVEN
	// Update flag
	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpatialWorkerFlags->SetWorkerFlag(TestWorkerFlagKey, TestWorkerFlagValue);
	// WHEN
	// Register callback
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnAnyWorkerFlagUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetAnyFlagUpdated);
	SpatialWorkerFlags->RegisterAndInvokeAnyFlagUpdatedCallback(WorkerFlagDelegate);
	// THEN
	TestEqual("Delegate Function was called once", SpyObj->GetTimesFlagUpdated(), 1);
	// Add test flag 2
	SpatialWorkerFlags->SetWorkerFlag(TestWorkerFlagKey2, TestWorkerFlagValue2);
	TestEqual("Delegate Function was called twice", SpyObj->GetTimesFlagUpdated(), 2);
	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_no_flags_WHEN_registering_and_invoking_an_any_flag_update_delegate_THEN_delegate_is_not_invoked)
{
	// GIVEN
	// No flags
	// WHEN
	// Register callback
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnAnyWorkerFlagUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetAnyFlagUpdated);
	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpatialWorkerFlags->RegisterAndInvokeAnyFlagUpdatedCallback(WorkerFlagDelegate);
	// THEN
	TestEqual("Delegate Function was not called", SpyObj->GetTimesFlagUpdated(), 0);
	return true;
}

// a flag update
SPATIALWORKERFLAGS_TEST(GIVEN_a_registered_flag_update_delegate_WHEN_the_worker_flag_updates_THEN_delegate_is_invoked)
{
	// GIVEN
	// Register callback
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnWorkerFlagUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetFlagUpdated);
	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpatialWorkerFlags->RegisterFlagUpdatedCallback(TestWorkerFlagKey, WorkerFlagDelegate);
	// WHEN
	// Update test flag
	SpatialWorkerFlags->SetWorkerFlag(TestWorkerFlagKey, TestWorkerFlagValue);
	// Update test flag again
	SpatialWorkerFlags->SetWorkerFlag(TestWorkerFlagKey, TestWorkerFlagValue);
	// THEN
	TestEqual("Delegate Function was called twice", SpyObj->GetTimesFlagUpdated(), 2);
	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_a_registered_flag_update_delegate_WHEN_a_different_worker_flag_updates_THEN_delegate_is_not_invoked)
{
	// GIVEN
	// Register callback
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnWorkerFlagUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetFlagUpdated);

	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpatialWorkerFlags->RegisterFlagUpdatedCallback(TestWorkerFlagKey, WorkerFlagDelegate);

	// WHEN
	// Add a different test flag
	SpatialWorkerFlags->SetWorkerFlag(TestWorkerFlagKey2, TestWorkerFlagValue2);

	// Update different test flag again
	SpatialWorkerFlags->SetWorkerFlag(TestWorkerFlagKey2, TestWorkerFlagValue2);

	// THEN
	TestEqual("Delegate Function was not called", SpyObj->GetTimesFlagUpdated(), 0);

	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_a_registered_flag_update_delegate_WHEN_unregistered_the_delegate_THEN_delegate_is_not_invoked)
{
	// GIVEN
	// Register callback
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnWorkerFlagUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetFlagUpdated);
	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpatialWorkerFlags->RegisterFlagUpdatedCallback(TestWorkerFlagKey, WorkerFlagDelegate);

	// Add test flag
	SpatialWorkerFlags->SetWorkerFlag(TestWorkerFlagKey, TestWorkerFlagValue);

	TestEqual("Delegate Function was called", SpyObj->GetTimesFlagUpdated(), 1);

	// WHEN
	// Unregister callback
	SpatialWorkerFlags->UnregisterFlagUpdatedCallback(TestWorkerFlagKey, WorkerFlagDelegate);

	// Update test flag
	SpatialWorkerFlags->SetWorkerFlag(TestWorkerFlagKey, TestWorkerFlagValue);

	// THEN
	TestEqual("Delegate Function was called only once", SpyObj->GetTimesFlagUpdated(), 1);

	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_an_updated_flag_WHEN_registering_and_invoking_flag_update_delegate_THEN_delegate_is_invoked)
{
	// GIVEN
	// Add test flag
	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpatialWorkerFlags->SetWorkerFlag(TestWorkerFlagKey, TestWorkerFlagValue);

	// WHEN
	// Register callback
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnWorkerFlagUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetFlagUpdated);
	SpatialWorkerFlags->RegisterAndInvokeFlagUpdatedCallback(TestWorkerFlagKey, WorkerFlagDelegate);
	// THEN
	TestEqual("Delegate Function was called", SpyObj->GetTimesFlagUpdated(), 1);
	// Update test flag
	SpatialWorkerFlags->SetWorkerFlag(TestWorkerFlagKey, TestWorkerFlagValue);
	TestEqual("Delegate Function was called twice", SpyObj->GetTimesFlagUpdated(), 2);
	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_no_flags_WHEN_registering_and_invoking_flag_update_delegate_THEN_delegate_is_not_invoked)
{
	// GIVEN
	// No flags

	// WHEN
	// Register callback
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnWorkerFlagUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetFlagUpdated);

	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpatialWorkerFlags->RegisterAndInvokeFlagUpdatedCallback(TestWorkerFlagKey, WorkerFlagDelegate);

	// THEN
	TestEqual("Delegate Function was not called", SpyObj->GetTimesFlagUpdated(), 0);

	// Add a different test flag
	SpatialWorkerFlags->SetWorkerFlag(TestWorkerFlagKey2, TestWorkerFlagValue2);

	TestEqual("Delegate Function was not called", SpyObj->GetTimesFlagUpdated(), 0);

	return true;
}

// Callback that unregister itself
// Defined this function here so the relevant code can be read together with the test
void UWorkerFlagsTestSpyObject::SetFlagUpdatedAndUnregisterCallback(const FString& FlagName, const FString& FlagValue)
{
	SetFlagUpdated(FlagName, FlagValue);
	SpatialWorkerFlags->UnregisterFlagUpdatedCallback(FlagName, WorkerFlagDelegate);
}

SPATIALWORKERFLAGS_TEST(
	GIVEN_a_registered_flag_update_delegate_that_unregisters_delegate_WHEN_the_worker_flag_updates_THEN_delegate_is_invoked_and_the_delegate_is_unregistered)
{
	// GIVEN
	// Register callback
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	SpyObj->SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpyObj->WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetFlagUpdatedAndUnregisterCallback);

	SpyObj->SpatialWorkerFlags->RegisterFlagUpdatedCallback(TestWorkerFlagKey, SpyObj->WorkerFlagDelegate);

	// WHEN
	// Update test flag
	SpyObj->SpatialWorkerFlags->SetWorkerFlag(TestWorkerFlagKey, TestWorkerFlagValue);

	// Update test flag again
	SpyObj->SpatialWorkerFlags->SetWorkerFlag(TestWorkerFlagKey, TestWorkerFlagValue);

	// THEN
	TestEqual("Delegate Function was called only once", SpyObj->GetTimesFlagUpdated(), 1);

	return true;
}
