// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "Interop/SpatialWorkerFlags.h"
#include "DummyObject.h"

#include "TestDefinitions.h"

#define SPATIALWORKERFLAGS_TEST(TestName) \
	GDK_TEST(Core, SpatialWorkerFlags, TestName)

namespace
{
	Worker_FlagUpdateOp Create_Worker_FlagUpdateOp(const char* name, const char* value)
	{
		Worker_FlagUpdateOp op = {};
		op.name = name;
		op.value = value;

		return op;
	}
}

SPATIALWORKERFLAGS_TEST(GIVEN_a_flagUpdate_op_WHEN_adding_a_worker_flag_THEN_flag_added)
{
	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	//add test flag
	Worker_FlagUpdateOp opAddFlag = Create_Worker_FlagUpdateOp("test", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(opAddFlag);

	FString OutValue;
	TestTrue("Flag added in the WorkerFlags map: ", SpatialWorkerFlags->GetWorkerFlag("test", OutValue));

	return true;
}


SPATIALWORKERFLAGS_TEST(GIVEN_a_flagUpdate_op_WHEN_removing_a_worker_flag_THEN_flag_removed)
{
	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	//add test flag
	Worker_FlagUpdateOp opAddFlag = Create_Worker_FlagUpdateOp("test", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(opAddFlag);

	FString OutValue;
	TestTrue("Flag added in the WorkerFlags map: ", SpatialWorkerFlags->GetWorkerFlag("test", OutValue));

	//remove test flag
	Worker_FlagUpdateOp opRemoveFlag = Create_Worker_FlagUpdateOp("test", nullptr);
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(opRemoveFlag);

	TestFalse("Flag removed from the WorkerFlags map: ", SpatialWorkerFlags->GetWorkerFlag("test", OutValue));

	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_a_bound_delegate_WHEN_a_worker_flag_updates_THEN_bound_function_invoked)
{
	UDummyObject* DummyObj = NewObject<UDummyObject>();
	FOnWorkerFlagsUpdatedBP workerFlagDelegate;
	workerFlagDelegate.BindDynamic(DummyObj, &UDummyObject::SetFlagUpdated);

	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpatialWorkerFlags->BindToOnWorkerFlagsUpdated(workerFlagDelegate);

	//add test flag
	Worker_FlagUpdateOp opAddFlag = Create_Worker_FlagUpdateOp("test", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(opAddFlag);

	TestTrue("Delegate Function was called", DummyObj->isFlagUpdated);

	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_a_bound_delegate_WHEN_unbind_the_delegate_THEN_bound_function_is_not_invoked)
{
	UDummyObject* DummyObj = NewObject<UDummyObject>();
	FOnWorkerFlagsUpdatedBP workerFlagDelegate;
	workerFlagDelegate.BindDynamic(DummyObj, &UDummyObject::SetFlagUpdated);

	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	SpatialWorkerFlags->BindToOnWorkerFlagsUpdated(workerFlagDelegate);
	//add test flag
	Worker_FlagUpdateOp opAddFlag = Create_Worker_FlagUpdateOp("test", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(opAddFlag);

	TestTrue("Delegate Function was called", DummyObj->isFlagUpdated);

	SpatialWorkerFlags->UnbindFromOnWorkerFlagsUpdated(workerFlagDelegate);

	//update test flag
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(opAddFlag);

	TestTrue("Delegate Function was called only once", DummyObj->timesUpdated == 1);

	return true;
}

