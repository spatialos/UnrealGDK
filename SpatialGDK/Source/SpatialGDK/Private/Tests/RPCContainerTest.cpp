// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestDefinitions.h"

#include "Utils/RPCContainer.h"
#include "Schema/RPCPayload.h"
#include "MockObject.h"

#include "Core.h"

#if WITH_DEV_AUTOMATION_TESTS

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

TEST(FRPCContainer, GIVEN_a_container_WHEN_nothing_has_been_added_THEN_nothing_is_queued)
{
	UMockObject* TargetObject = NewObject<UMockObject>();

	FPendingRPCParamsPtr Params = CreateMockParameters(TargetObject);
	FRPCContainer RPCs;
	FUnrealObjectRef ObjecRef = Params->ObjectRef;

	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, ReliableType));

    return true;
}

TEST(FRPCContainer, GIVEN_a_container_WHEN_one_value_has_been_added_THEN_it_is_queued)
{
	UMockObject* TargetObject = NewObject<UMockObject>();

	FPendingRPCParamsPtr Params = CreateMockParameters(TargetObject);
	FRPCContainer RPCs;
	FUnrealObjectRef ObjecRef = Params->ObjectRef;
	RPCs.QueueRPC(MoveTemp(Params), ReliableType);

	TestTrue("Queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, ReliableType));

    return true;
}

TEST(FRPCContainer, GIVEN_a_container_WHEN_one_value_has_been_added_and_processed_THEN_nothing_is_queued)
{
	UMockObject* TargetObject = NewObject<UMockObject>();

	FPendingRPCParamsPtr Params = CreateMockParameters(TargetObject);
	FRPCContainer RPCs;
	FUnrealObjectRef ObjecRef = Params->ObjectRef;

	RPCs.QueueRPC(MoveTemp(Params), ReliableType);

	FProcessRPCDelegate Delegate;
	Delegate.BindUObject(TargetObject, &UMockObject::ProcessRPC);
	RPCs.ProcessRPCs(Delegate);

	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, ReliableType));

    return true;
}

TEST(FRPCContainer, GIVEN_a_container_WHEN_two_values_of_same_type_have_been_added_THEN_they_are_queued)
{
	UMockObject* TargetObject = NewObject<UMockObject>();
	FPendingRPCParamsPtr Params1 = CreateMockParameters(TargetObject);
	FUnrealObjectRef ObjecRef1 = Params1->ObjectRef;
	FPendingRPCParamsPtr Params2 = CreateMockParameters(TargetObject);
	FUnrealObjectRef ObjecRef2 = Params2->ObjectRef;

	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(Params1), UnreliableType);
	RPCs.QueueRPC(MoveTemp(Params2), UnreliableType);

	TestTrue("Queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef1.Entity, UnreliableType));

    return true;
}

TEST(FRPCContainer, GIVEN_a_container_WHEN_two_values_of_same_type_have_been_added_and_processed_THEN_nothing_is_queued)
{
	UMockObject* TargetObject = NewObject<UMockObject>();
	FPendingRPCParamsPtr Params1 = CreateMockParameters(TargetObject);
	FUnrealObjectRef ObjecRef1 = Params1->ObjectRef;
	FPendingRPCParamsPtr Params2 = CreateMockParameters(TargetObject);
	FUnrealObjectRef ObjecRef2 = Params2->ObjectRef;

	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(Params1), UnreliableType);
	RPCs.QueueRPC(MoveTemp(Params2), UnreliableType);

	FProcessRPCDelegate Delegate;
	Delegate.BindUObject(TargetObject, &UMockObject::ProcessRPC);
	RPCs.ProcessRPCs(Delegate);

	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef1.Entity, UnreliableType));

    return true;
}

TEST(FRPCContainer, GIVEN_a_container_WHEN_two_values_of_different_type_have_been_added_THEN_they_are_queued)
{
	UMockObject* TargetObject = NewObject<UMockObject>();

	FPendingRPCParamsPtr ParamsUnreliable = CreateMockParameters(TargetObject);
	FUnrealObjectRef ObjecRefUnreliable = ParamsUnreliable->ObjectRef;

	FPendingRPCParamsPtr ParamsReliable = CreateMockParameters(TargetObject);
	FUnrealObjectRef ObjecRefReliable = ParamsReliable->ObjectRef;

	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(ParamsUnreliable), UnreliableType);
	RPCs.QueueRPC(MoveTemp(ParamsReliable), ReliableType);

	TestTrue("Queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRefUnreliable.Entity, UnreliableType));
	TestTrue("Queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRefReliable.Entity, ReliableType));

    return true;
}

TEST(FRPCContainer, GIVEN_a_container_WHEN_two_values_of_different_type_have_been_added_and_processed_THEN_nothing_is_queued)
{
	UMockObject* TargetObject = NewObject<UMockObject>();

	FPendingRPCParamsPtr ParamsUnreliable = CreateMockParameters(TargetObject);
	FUnrealObjectRef ObjecRefUnreliable = ParamsUnreliable->ObjectRef;

	FPendingRPCParamsPtr ParamsReliable = CreateMockParameters(TargetObject);
	FUnrealObjectRef ObjecRefReliable = ParamsReliable->ObjectRef;

	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(ParamsUnreliable), UnreliableType);
	RPCs.QueueRPC(MoveTemp(ParamsReliable), ReliableType);

	FProcessRPCDelegate Delegate;
	Delegate.BindUObject(TargetObject, &UMockObject::ProcessRPC);
	RPCs.ProcessRPCs(Delegate);

	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRefUnreliable.Entity, UnreliableType));
	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRefReliable.Entity, ReliableType));

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
