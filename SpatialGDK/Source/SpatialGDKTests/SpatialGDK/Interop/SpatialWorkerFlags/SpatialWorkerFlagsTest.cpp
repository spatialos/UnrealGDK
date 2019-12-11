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
	FString OutValue;

	//add test flag
	Worker_FlagUpdateOp op = Create_Worker_FlagUpdateOp("test", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(op);

	TestTrue("Flag added in the WorkerFlags map: ", SpatialWorkerFlags->GetWorkerFlag("test", OutValue));

	return true;
}


SPATIALWORKERFLAGS_TEST(GIVEN_a_flagUpdate_op_WHEN_removing_a_worker_flag_THEN_flag_removed)
{
	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();
	FString OutValue;

	//add test flag
	Worker_FlagUpdateOp op = Create_Worker_FlagUpdateOp("test", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(op);

	//remove test flag
	Worker_FlagUpdateOp op1 = Create_Worker_FlagUpdateOp("test", nullptr);
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(op1);
	
	TestFalse("Flag removed from the WorkerFlags map: ", SpatialWorkerFlags->GetWorkerFlag("test", OutValue));

	return true;
}

SPATIALWORKERFLAGS_TEST(GIVEN_a_bound_delegate_WHEN_a_worker_flag_updates_THEN_bound_function_invoked)
{
	UDummyObject* DummyObj = NewObject<UDummyObject>();

	USpatialWorkerFlags* SpatialWorkerFlags = NewObject<USpatialWorkerFlags>();

	FOnWorkerFlagsUpdatedBP workerFlagDelegate;
	workerFlagDelegate.BindDynamic(DummyObj, &UDummyObject::SetFlagUpdated);

	SpatialWorkerFlags->BindToOnWorkerFlagsUpdated(workerFlagDelegate);

	//add test flag
	Worker_FlagUpdateOp op = Create_Worker_FlagUpdateOp("test", "10");
	SpatialWorkerFlags->ApplyWorkerFlagUpdate(op);

	TestTrue("Delegate Function was called", DummyObj->isFlagUpdated);

	return true;
}
