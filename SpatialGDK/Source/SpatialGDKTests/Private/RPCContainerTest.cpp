// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestDefinitions.h"

#include "DummyObject.h"
#include "SpyObject.h"

#include <Utils/RPCContainer.h>
#include <Schema/RPCPayload.h>

#include <Core.h>

#if WITH_DEV_AUTOMATION_TESTS

#define RPCCONTAINER_TEST(TestName) \
	TEST(FRPCContainer, TestName)

using namespace SpatialGDK;

namespace
{
	ESchemaComponentType AnySchemaComponentType = ESchemaComponentType::SCHEMA_ClientReliableRPC;
	ESchemaComponentType AnyOtherSchemaComponentType = ESchemaComponentType::SCHEMA_ClientUnreliableRPC;

	FUnrealObjectRef GenerateObjectRef(UObject* TargetObject)
	{
		return FUnrealObjectRef{ Worker_EntityId(TargetObject), 0 };
	}

	uint32 GeneratePayloadFunctionIndex()
	{
		static uint32 freeIndex = 0;
		return freeIndex++;
	}

	FPendingRPCParamsPtr CreateMockParameters(UObject* TargetObject, ESchemaComponentType Type)
	{
		// Use PayloadData as a place to store RPC type
		RPCPayload Payload(0, GeneratePayloadFunctionIndex(), TypeToArray(Type));
		int ReliableRPCIndex = 0;

		FUnrealObjectRef ObjectRef = GenerateObjectRef(TargetObject);

		return MakeUnique<FPendingRPCParams>(ObjectRef, MoveTemp(Payload), ReliableRPCIndex);
	}
}

RPCCONTAINER_TEST(GIVEN_a_container_WHEN_nothing_has_been_added_THEN_nothing_is_queued)
{
	UDummyObject* TargetObject = NewObject<UDummyObject>();
	FUnrealObjectRef ObjecRef = GenerateObjectRef(TargetObject);
	FPendingRPCParamsPtr Params = CreateMockParameters(TargetObject, AnySchemaComponentType);
	FRPCContainer RPCs;

	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, AnySchemaComponentType));

    return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_WHEN_one_value_has_been_added_THEN_it_is_queued)
{
	UDummyObject* TargetObject = NewObject<UDummyObject>();
	FUnrealObjectRef ObjecRef = GenerateObjectRef(TargetObject);
	FPendingRPCParamsPtr Params = CreateMockParameters(TargetObject, AnySchemaComponentType);
	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(Params), AnySchemaComponentType);

	TestTrue("Queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, AnySchemaComponentType));

    return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_WHEN_multiple_values_of_same_type_have_been_added_THEN_they_are_queued)
{
	UDummyObject* TargetObject = NewObject<UDummyObject>();
	FUnrealObjectRef ObjecRef = GenerateObjectRef(TargetObject);
	FPendingRPCParamsPtr Params1 = CreateMockParameters(TargetObject, AnyOtherSchemaComponentType);
	FPendingRPCParamsPtr Params2 = CreateMockParameters(TargetObject, AnyOtherSchemaComponentType);
	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(Params1), AnyOtherSchemaComponentType);
	RPCs.QueueRPC(MoveTemp(Params2), AnyOtherSchemaComponentType);

	TestTrue("Queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, AnyOtherSchemaComponentType));

    return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_storing_one_value_WHEN_processed_once_THEN_nothing_is_queued)
{
	UDummyObject* TargetObject = NewObject<UDummyObject>();
	FUnrealObjectRef ObjecRef = GenerateObjectRef(TargetObject);
	FPendingRPCParamsPtr Params = CreateMockParameters(TargetObject, AnySchemaComponentType);
	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(Params), AnySchemaComponentType);

	FProcessRPCDelegate Delegate;
	Delegate.BindUObject(TargetObject, &UDummyObject::ProcessRPC);
	RPCs.ProcessRPCs(Delegate);

	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, AnySchemaComponentType));

    return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_storinng_multiple_values_of_same_type_WHEN_processed_once_THEN_nothing_is_queued)
{
	UDummyObject* TargetObject = NewObject<UDummyObject>();
	FUnrealObjectRef ObjecRef = GenerateObjectRef(TargetObject);
	FPendingRPCParamsPtr Params1 = CreateMockParameters(TargetObject, AnyOtherSchemaComponentType);
	FPendingRPCParamsPtr Params2 = CreateMockParameters(TargetObject, AnyOtherSchemaComponentType);
	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(Params1), AnyOtherSchemaComponentType);
	RPCs.QueueRPC(MoveTemp(Params2), AnyOtherSchemaComponentType);

	FProcessRPCDelegate Delegate;
	Delegate.BindUObject(TargetObject, &UDummyObject::ProcessRPC);
	RPCs.ProcessRPCs(Delegate);

	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, AnyOtherSchemaComponentType));

    return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_WHEN_multiple_values_of_different_type_have_been_added_THEN_they_are_queued)
{
	UDummyObject* TargetObject = NewObject<UDummyObject>();

	FPendingRPCParamsPtr ParamsUnreliable = CreateMockParameters(TargetObject, AnyOtherSchemaComponentType);
	FUnrealObjectRef ObjecRefUnreliable = ParamsUnreliable->ObjectRef;

	FPendingRPCParamsPtr ParamsReliable = CreateMockParameters(TargetObject, AnySchemaComponentType);
	FUnrealObjectRef ObjecRefReliable = ParamsReliable->ObjectRef;

	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(ParamsUnreliable), AnyOtherSchemaComponentType);
	RPCs.QueueRPC(MoveTemp(ParamsReliable), AnySchemaComponentType);

	TestTrue("Queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRefUnreliable.Entity, AnyOtherSchemaComponentType));
	TestTrue("Queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRefReliable.Entity, AnySchemaComponentType));

    return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_storing_multiple_values_of_different_type_WHEN_processed_once_THEN_nothing_is_queued)
{
	UDummyObject* TargetObject = NewObject<UDummyObject>();
	FUnrealObjectRef ObjecRef = GenerateObjectRef(TargetObject);
	FPendingRPCParamsPtr ParamsUnreliable = CreateMockParameters(TargetObject, AnyOtherSchemaComponentType);
	FPendingRPCParamsPtr ParamsReliable = CreateMockParameters(TargetObject, AnySchemaComponentType);

	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(ParamsUnreliable), AnyOtherSchemaComponentType);
	RPCs.QueueRPC(MoveTemp(ParamsReliable), AnySchemaComponentType);

	FProcessRPCDelegate Delegate;
	Delegate.BindUObject(TargetObject, &UDummyObject::ProcessRPC);
	RPCs.ProcessRPCs(Delegate);

	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, AnyOtherSchemaComponentType));
	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, AnySchemaComponentType));

    return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_storing_multiple_values_of_different_type_WHEN_processed_once_THEN_values_have_been_processed_in_order)
{
	USpyObject* TargetObject = NewObject<USpyObject>();
	FUnrealObjectRef ObjectRef = GenerateObjectRef(TargetObject);
	FRPCContainer RPCs;

	TMap<ESchemaComponentType, TArray<uint32>> RPCIndices;

	for (int i = 0; i < 4; ++i)
	{
		FPendingRPCParamsPtr ParamsUnreliable = CreateMockParameters(TargetObject, AnyOtherSchemaComponentType);
		FPendingRPCParamsPtr ParamsReliable = CreateMockParameters(TargetObject, AnySchemaComponentType);

		RPCIndices.FindOrAdd(AnyOtherSchemaComponentType).Push(ParamsUnreliable->Payload.Index);
		RPCIndices.FindOrAdd(AnySchemaComponentType).Push(ParamsReliable->Payload.Index);

		RPCs.QueueRPC(MoveTemp(ParamsUnreliable), AnyOtherSchemaComponentType);
		RPCs.QueueRPC(MoveTemp(ParamsReliable), AnySchemaComponentType);
	}

	FProcessRPCDelegate Delegate;
	Delegate.BindUObject(TargetObject, &USpyObject::ProcessRPC);
	RPCs.ProcessRPCs(Delegate);

	bool bProcessedInOrder = true;
	for (const auto& processedIndicesOfType : TargetObject->ProcessedRPCIndices)
	{
		TArray<uint32>* storedIndicesOfType = RPCIndices.Find(processedIndicesOfType.Key);
		if (!storedIndicesOfType)
		{
			bProcessedInOrder = false;
			break;
		}

		if (processedIndicesOfType.Value.Num() != (*storedIndicesOfType).Num())
		{
			bProcessedInOrder = false;
			break;
		}

		const int32 numIndices = processedIndicesOfType.Value.Num();

		for (int i = 0; i < numIndices; ++i)
		{
			if (processedIndicesOfType.Value[i] != (*storedIndicesOfType)[i])
			{
				bProcessedInOrder = false;
				break;
			}
		}
	}

	TestTrue("Queued RPCs have been processed in order", bProcessedInOrder);

    return true;
}
#endif // WITH_DEV_AUTOMATION_TESTS
