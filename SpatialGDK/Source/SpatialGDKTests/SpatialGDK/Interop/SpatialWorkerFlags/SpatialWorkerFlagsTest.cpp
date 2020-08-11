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

SPATIALWORKERFLAGS_TEST(GIVEN_a_bound_delegate_WHEN_a_worker_flag_updates_THEN_bound_function_invoked)
{
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnWorkerFlagsUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetFlagUpdated);

	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpatialWorkerFlags->BindToOnWorkerFlagsUpdated(WorkerFlagDelegate);

	// Add test flag
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp("test", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	TestTrue("Delegate Function was called", SpyObj->GetTimesFlagUpdated() == 1);

	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_a_bound_delegate_WHEN_unbind_the_delegate_THEN_bound_function_is_not_invoked)
{
	UWorkerFlagsTestSpyObject* SpyObj = NewObject<UWorkerFlagsTestSpyObject>();
	FOnWorkerFlagsUpdatedBP WorkerFlagDelegate;
	WorkerFlagDelegate.BindDynamic(SpyObj, &UWorkerFlagsTestSpyObject::SetFlagUpdated);

	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpatialWorkerFlags->BindToOnWorkerFlagsUpdated(WorkerFlagDelegate);
	// Add test flag
	Worker_FlagUpdateOp OpAddFlag = CreateWorkerFlagUpdateOp("test", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	TestTrue("Delegate Function was called", SpyObj->GetTimesFlagUpdated() == 1);

	SpatialWorkerFlags->UnbindFromOnWorkerFlagsUpdated(WorkerFlagDelegate);

	// Update test flag
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(OpAddFlag);

	TestTrue("Delegate Function was called only once", SpyObj->GetTimesFlagUpdated() == 1);

	return true;
}
