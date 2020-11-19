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
	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	// Add test flag
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp("test", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	FString OutFlagValue;
	TestTrue("Flag added in the WorkerFlags map: ", SpatialWorkerFlags->GetWorkerFlag("test", OutFlagValue));

	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_a_flagUpdate_op_WHEN_removing_a_worker_flag_THEN_flag_removed)
{
	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	// Add test flag
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp("test", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	FString OutFlagValue;
	TestTrue("Flag added in the WorkerFlags map: ", SpatialWorkerFlags->GetWorkerFlag("test", OutFlagValue));

	// Remove test flag
	Worker_FlagUpdateOp OpRemoveFlag = CreateWorkerFlagUpdateOp("test", nullptr);
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpRemoveFlag);

	TestFalse("Flag removed from the WorkerFlags map: ", SpatialWorkerFlags->GetWorkerFlag("test", OutFlagValue));

	return true;
}

// delegates

// any flag

SPATIALWORKERFLAGS_TEST(GIVEN_a_registered_any_flag_update_delegate_WHEN_any_worker_flag_updates_THEN_delegate_invoked)
{
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnAnyWorkerFlagUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetAnyFlagUpdated);

	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpatialWorkerFlags->RegisterAnyFlagUpdatedCallback(WorkerFlagDelegate);

	// Add test flag
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp("test", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	// Add test flag 2
	Worker_FlagUpdateOp OpAddFlag2 = CreateWorkerFlagUpdateOp("test2", "20");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag2);

	TestTrue("Delegate Function was called twice", SpyObj->GetTimesFlagUpdated() == 2);

	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_a_registered_any_flag_update_delegate_WHEN_unregistering_the_delegate_THEN_delegate_is_not_invoked)
{
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnAnyWorkerFlagUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetAnyFlagUpdated);

	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpatialWorkerFlags->RegisterAnyFlagUpdatedCallback(WorkerFlagDelegate);
	// Add test flag
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp("test", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	TestTrue("Delegate Function was called", SpyObj->GetTimesFlagUpdated() == 1);

	SpatialWorkerFlags->UnregisterAnyFlagUpdatedCallback(WorkerFlagDelegate);

	// Update test flag
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	TestTrue("Delegate Function was called only once", SpyObj->GetTimesFlagUpdated() == 1);

	return true;
}

// any flag invoke

SPATIALWORKERFLAGS_TEST(GIVEN_an_updated_flag_WHEN_registering_and_invoking_an_any_flag_update_delegate_THEN_delegate_is_invoked)
{
	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();

	// Add test flag
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp("test", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	// register callback
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnAnyWorkerFlagUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetAnyFlagUpdated);

	SpatialWorkerFlags->RegisterAndInvokeAnyFlagUpdatedCallback(WorkerFlagDelegate);
	// should invoke immediately
	TestTrue("Delegate Function was called once", SpyObj->GetTimesFlagUpdated() == 1);

	// Add test flag 2
	Worker_FlagUpdateOp OpAddFlag2 = CreateWorkerFlagUpdateOp("test2", "20");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag2);

	TestTrue("Delegate Function was called twice", SpyObj->GetTimesFlagUpdated() == 2);

	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_no_flags_WHEN_registering_and_invoking_an_any_flag_update_delegate_THEN_delegate_is_not_invoked)
{
	// register callback
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnAnyWorkerFlagUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetAnyFlagUpdated);

	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpatialWorkerFlags->RegisterAndInvokeAnyFlagUpdatedCallback(WorkerFlagDelegate);
	// should not invoke
	TestTrue("Delegate Function was not called", SpyObj->GetTimesFlagUpdated() == 0);

	return true;
}

// a flag update

SPATIALWORKERFLAGS_TEST(GIVEN_a_registered_flag_update_delegate_WHEN_the_worker_flag_updates_THEN_delegate_is_invoked)
{
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnWorkerFlagUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetFlagUpdated);

	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	const FString TestFlag = TEXT("test");
	SpatialWorkerFlags->RegisterFlagUpdatedCallback(TestFlag, WorkerFlagDelegate);

	// Add test flag
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp(TCHAR_TO_ANSI(*TestFlag), "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	// Update test flag
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	TestTrue("Delegate Function was called twice", SpyObj->GetTimesFlagUpdated() == 2);

	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_a_registered_flag_update_delegate_WHEN_a_different_worker_flag_updates_THEN_delegate_is_not_invoked)
{
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnWorkerFlagUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetFlagUpdated);

	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpatialWorkerFlags->RegisterFlagUpdatedCallback("test", WorkerFlagDelegate);

	// Add a different test flag
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp("test2", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	// Update test flag
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	TestTrue("Delegate Function was not called", SpyObj->GetTimesFlagUpdated() == 0);

	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_a_registered_flag_update_delegate_WHEN_unregistered_the_delegate_THEN_delegate_is_not_invoked)
{
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnWorkerFlagUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetFlagUpdated);

	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	const FString TestFlag = TEXT("test");
	SpatialWorkerFlags->RegisterFlagUpdatedCallback(TestFlag, WorkerFlagDelegate);
	// Add test flag
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp(TCHAR_TO_ANSI(*TestFlag), "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	TestTrue("Delegate Function was called", SpyObj->GetTimesFlagUpdated() == 1);

	SpatialWorkerFlags->UnregisterFlagUpdatedCallback(TCHAR_TO_ANSI(*TestFlag), WorkerFlagDelegate);

	// Update test flag
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	TestTrue("Delegate Function was called only once", SpyObj->GetTimesFlagUpdated() == 1);

	return true;
}

// a flag invoke

SPATIALWORKERFLAGS_TEST(GIVEN_an_updated_flag_WHEN_registering_and_invoking_flag_update_delegate_THEN_delegate_is_invoked)
{
	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();

	// Add test flag
	const FString TestFlag = TEXT("test");
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp(TCHAR_TO_ANSI(*TestFlag), "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	// register callback
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnWorkerFlagUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetFlagUpdated);

	// should invoke immediately
	SpatialWorkerFlags->RegisterAndInvokeFlagUpdatedCallback(TestFlag, WorkerFlagDelegate);

	TestTrue("Delegate Function was called", SpyObj->GetTimesFlagUpdated() == 1);

	// Update test flag
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	TestTrue("Delegate Function was called twice", SpyObj->GetTimesFlagUpdated() == 2);

	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_no_flags_WHEN_registering_and_invoking_flag_update_delegate_THEN_delegate_is_not_invoked)
{
	// register callback
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnWorkerFlagUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetFlagUpdated);

	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpatialWorkerFlags->RegisterAndInvokeFlagUpdatedCallback("test", WorkerFlagDelegate);
	// should not invoke
	TestTrue("Delegate Function was not called", SpyObj->GetTimesFlagUpdated() == 0);

	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp("test2", "10");

	// should not invoke
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	TestTrue("Delegate Function was not called", SpyObj->GetTimesFlagUpdated() == 0);

	return true;
}
