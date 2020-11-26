// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialWorkerFlags.h"

#include "Tests/TestDefinitions.h"
#include "WorkerFlagsTestSpyObject.h"

#define SPATIALWORKERFLAGS_TEST(TestName) GDK_TEST(Core, SpatialWorkerFlags, TestName)

namespace
{
Worker_FlagUpdateOp CreateWorkerFlagUpdateOp(const char* FlagName, const char* FlagValue)
{
	Worker_FlagUpdateOp Op = {};
	Op.name = FlagName;
	Op.value = FlagValue;

	return Op;
}
} // anonymous namespace

SPATIALWORKERFLAGS_TEST(GIVEN_a_flagUpdate_op_WHEN_adding_a_worker_flag_THEN_flag_added)
{
	// GIVEN
	// Add test flag
	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp("test", "10");

	// WHEN
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	// THEN
	FString OutFlagValue;
	TestTrue("Flag added in the WorkerFlags map: ", SpatialWorkerFlags->GetWorkerFlag("test", OutFlagValue));

	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_a_flagUpdate_op_WHEN_removing_a_worker_flag_THEN_flag_removed)
{
	// GIVEN
	// Add test flag
	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp("test", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);
	FString OutFlagValue;
	TestTrue("Flag added in the WorkerFlags map: ", SpatialWorkerFlags->GetWorkerFlag("test", OutFlagValue));

	// WHEN
	// Remove test flag
	Worker_FlagUpdateOp OpRemoveFlag = CreateWorkerFlagUpdateOp("test", nullptr);
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpRemoveFlag);

	// THEN
	TestFalse("Flag removed from the WorkerFlags map: ", SpatialWorkerFlags->GetWorkerFlag("test", OutFlagValue));

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
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp("test", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	// Update another flag
	Worker_FlagUpdateOp OpAddFlag2 = CreateWorkerFlagUpdateOp("test2", "20");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag2);

	// THEN
	TestTrue("Delegate Function was called twice", SpyObj->GetTimesFlagUpdated() == 2);

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
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp("test", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	TestTrue("Delegate Function was called", SpyObj->GetTimesFlagUpdated() == 1);

	// WHEN
	// Unregister callback
	SpatialWorkerFlags->UnregisterAnyFlagUpdatedCallback(WorkerFlagDelegate);

	// Update test flag again
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	// THEN
	TestTrue("Delegate Function was called only once", SpyObj->GetTimesFlagUpdated() == 1);

	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_an_updated_flag_WHEN_registering_and_invoking_an_any_flag_update_delegate_THEN_delegate_is_invoked)
{
	// GIVEN
	// Update flag
	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();

	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp("test", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	// WHEN
	// Register callback
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnAnyWorkerFlagUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetAnyFlagUpdated);
	SpatialWorkerFlags->RegisterAndInvokeAnyFlagUpdatedCallback(WorkerFlagDelegate);

	// THEN
	TestTrue("Delegate Function was called once", SpyObj->GetTimesFlagUpdated() == 1);

	// Add test flag 2
	Worker_FlagUpdateOp OpAddFlag2 = CreateWorkerFlagUpdateOp("test2", "20");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag2);

	TestTrue("Delegate Function was called twice", SpyObj->GetTimesFlagUpdated() == 2);

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
	TestTrue("Delegate Function was not called", SpyObj->GetTimesFlagUpdated() == 0);

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
	const FString TestFlagName = TEXT("test");
	SpatialWorkerFlags->RegisterFlagUpdatedCallback(TestFlagName, WorkerFlagDelegate);

	// WHEN
	// Update test flag
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp(TCHAR_TO_ANSI(*TestFlagName), "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	// Update test flag again
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	// THEN
	TestTrue("Delegate Function was called twice", SpyObj->GetTimesFlagUpdated() == 2);

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
	SpatialWorkerFlags->RegisterFlagUpdatedCallback("test", WorkerFlagDelegate);

	// WHEN
	// Add a different test flag
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp("test2", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	// Update different test flag again
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	// THEN
	TestTrue("Delegate Function was not called", SpyObj->GetTimesFlagUpdated() == 0);

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
	const FString TestFlagName = TEXT("test");
	SpatialWorkerFlags->RegisterFlagUpdatedCallback(TestFlagName, WorkerFlagDelegate);

	// Add test flag
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp(TCHAR_TO_ANSI(*TestFlagName), "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	TestTrue("Delegate Function was called", SpyObj->GetTimesFlagUpdated() == 1);

	// WHEN
	// Unregister callback
	SpatialWorkerFlags->UnregisterFlagUpdatedCallback(TCHAR_TO_ANSI(*TestFlagName), WorkerFlagDelegate);

	// Update test flag
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	// THEN
	TestTrue("Delegate Function was called only once", SpyObj->GetTimesFlagUpdated() == 1);

	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_an_updated_flag_WHEN_registering_and_invoking_flag_update_delegate_THEN_delegate_is_invoked)
{
	// GIVEN
	// Add test flag
	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	const FString TestFlagName = TEXT("test");
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp(TCHAR_TO_ANSI(*TestFlagName), "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	// WHEN
	// Register callback
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnWorkerFlagUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetFlagUpdated);
	SpatialWorkerFlags->RegisterAndInvokeFlagUpdatedCallback(TestFlagName, WorkerFlagDelegate);

	// THEN
	TestTrue("Delegate Function was called", SpyObj->GetTimesFlagUpdated() == 1);

	// Update test flag
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	TestTrue("Delegate Function was called twice", SpyObj->GetTimesFlagUpdated() == 2);

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
	SpatialWorkerFlags->RegisterAndInvokeFlagUpdatedCallback("test", WorkerFlagDelegate);

	// THEN
	TestTrue("Delegate Function was not called", SpyObj->GetTimesFlagUpdated() == 0);

	// Add a different test flag
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp("test2", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	TestTrue("Delegate Function was not called", SpyObj->GetTimesFlagUpdated() == 0);

	return true;
}

// Callback that unregister itself

// Defined this function here so the relevant code can be read together with the test
void UWorkerFlagsTestSpyObject::SetFlagUpdatedAndUnregisterCallback(const FString& FlagName, const FString& FlagValue)
{
	SetFlagUpdated(FlagName, FlagValue);
	SpatialWorkerFlags->UnregisterFlagUpdatedCallback(TCHAR_TO_ANSI(*FlagName), WorkerFlagDelegate);
}

SPATIALWORKERFLAGS_TEST(
	GIVEN_a_registered_flag_update_delegate_that_unregisters_delegate_WHEN_the_worker_flag_updates_THEN_delegate_is_invoked_and_the_delegate_is_unregistered)
{
	// GIVEN
	// Register callback
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	SpyObj->SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpyObj->WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetFlagUpdatedAndUnregisterCallback);

	const FString TestFlagName = TEXT("test");
	SpyObj->SpatialWorkerFlags->RegisterFlagUpdatedCallback(TestFlagName, SpyObj->WorkerFlagDelegate);

	// WHEN
	// Update test flag
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp(TCHAR_TO_ANSI(*TestFlagName), "10");
	SpyObj->SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	// Update test flag again
	SpyObj->SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	// THEN
	TestTrue("Delegate Function was called only once", SpyObj->GetTimesFlagUpdated() == 1);

	return true;
}
