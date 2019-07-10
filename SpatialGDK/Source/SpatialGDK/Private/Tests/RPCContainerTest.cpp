// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/RPCContainer.h"
#include "Schema/RPCPayload.h"
#include "MockObject.h"

#include "Core.h"

#if WITH_DEV_AUTOMATION_TESTS

#define TEST(TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestName, "SpatialGDK.EngineClasses."#TestName, EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter) \
	bool TestName::RunTest(const FString& Parameters)

using namespace SpatialGDK;

namespace
{
	static ESchemaComponentType ReliableType = ESchemaComponentType::SCHEMA_ClientReliableRPC;
	static ESchemaComponentType UnreliableType = ESchemaComponentType::SCHEMA_ClientUnreliableRPC;

	static Worker_EntityId GenerateEntityId()
	{
		static Worker_EntityId FreeId = 1;
		return FreeId++;
	}

	static FPendingRPCParamsPtr CreateMockParameters(UObject* TargetObject)
	{
		RPCPayload Payload(0, 0, TArray<uint8>{});
		int ReliableRPCIndex = 0;

		auto ObjectRef = FUnrealObjectRef{ GenerateEntityId(), 0 };

		return MakeUnique<FPendingRPCParams>(ObjectRef, MoveTemp(Payload), ReliableRPCIndex);
	}
}
/*
// TODO(Alex): 
// Create modules
// have to expose more
// add automation macro
TEST(GIVEN_an_empty_container_WHEN_an_RPC_is_added_THEN_blah)
{
	// Setup
	TestTrue();
	return true;
}
*/

TEST(FFRPCContainerTestOneQueuedRPC)
{
	UMockObject* TargetObject = NewObject<UMockObject>();

	FPendingRPCParamsPtr Params = CreateMockParameters(TargetObject);
	FRPCContainer RPCs;
	FUnrealObjectRef ObjecRef = Params->ObjectRef;

	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, ReliableType));

	RPCs.QueueRPC(MoveTemp(Params), ReliableType);

	TestTrue("Queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, ReliableType));

	FProcessRPCDelegate Delegate;
	Delegate.BindUObject(TargetObject, &UMockObject::ProcessRPC);
	RPCs.ProcessRPCs(Delegate);

	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, ReliableType));

    return true;
}

TEST(FFRPCContainerTwoQueuedRPCSameType)
{
	UMockObject* TargetObject = NewObject<UMockObject>();
	FPendingRPCParamsPtr Params1 = CreateMockParameters(TargetObject);
	FUnrealObjectRef ObjecRef1 = Params1->ObjectRef;

	FRPCContainer RPCs;

	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef1.Entity, UnreliableType));

	RPCs.QueueRPC(MoveTemp(Params1), UnreliableType);

	FPendingRPCParamsPtr Params2 = CreateMockParameters(TargetObject);
	FUnrealObjectRef ObjecRef2 = Params2->ObjectRef;

	RPCs.QueueRPC(MoveTemp(Params2), UnreliableType);

	TestTrue("Queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef1.Entity, UnreliableType));
	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef2.Entity, ReliableType));

	FProcessRPCDelegate Delegate;
	Delegate.BindUObject(TargetObject, &UMockObject::ProcessRPC);
	RPCs.ProcessRPCs(Delegate);

	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef1.Entity, UnreliableType));

    return true;
}

TEST(FFRPCContainerTwoQueuedRPCDifferentType)
{
	UMockObject* TargetObject = NewObject<UMockObject>();

	FPendingRPCParamsPtr ParamsUnreliable = CreateMockParameters(TargetObject);
	FUnrealObjectRef ObjecRefUnreliable = ParamsUnreliable->ObjectRef;

	FPendingRPCParamsPtr ParamsReliable = CreateMockParameters(TargetObject);
	FUnrealObjectRef ObjecRefReliable = ParamsReliable->ObjectRef;

	FRPCContainer RPCs;

	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRefUnreliable.Entity, UnreliableType));
	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRefUnreliable.Entity, ReliableType));

	RPCs.QueueRPC(MoveTemp(ParamsUnreliable), UnreliableType);
	RPCs.QueueRPC(MoveTemp(ParamsReliable), ReliableType);

	TestTrue("Queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRefUnreliable.Entity, UnreliableType));
	TestTrue("Queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRefReliable.Entity, ReliableType));

	FProcessRPCDelegate Delegate;
	Delegate.BindUObject(TargetObject, &UMockObject::ProcessRPC);
	RPCs.ProcessRPCs(Delegate);

	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRefUnreliable.Entity, UnreliableType));
	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRefReliable.Entity, ReliableType));

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
