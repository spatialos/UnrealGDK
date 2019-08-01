// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestDefinitions.h"

#include "ObjectDummy.h"
#include "ObjectSpy.h"

#include "Utils/RPCContainer.h"
#include "Schema/RPCPayload.h"

#include "CoreMinimal.h"

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
		static uint32 FreeIndex = 0;
		return FreeIndex++;
	}

	FPendingRPCParamsPtr CreateMockParameters(UObject* TargetObject, ESchemaComponentType Type)
	{
		// Use PayloadData as a place to store RPC type
		RPCPayload Payload(0, GeneratePayloadFunctionIndex(), SpyUtils::SchemaTypeToByteArray(Type));
		int ReliableRPCIndex = 0;

		FUnrealObjectRef ObjectRef = GenerateObjectRef(TargetObject);

		return MakeUnique<FPendingRPCParams>(ObjectRef, MoveTemp(Payload), ReliableRPCIndex);
	}
} // anonymous namespace

RPCCONTAINER_TEST(GIVEN_a_container_WHEN_nothing_has_been_added_THEN_nothing_is_queued)
{
	UObjectDummy* TargetObject = NewObject<UObjectDummy>();
	FUnrealObjectRef ObjecRef = GenerateObjectRef(TargetObject);
	FPendingRPCParamsPtr Params = CreateMockParameters(TargetObject, AnySchemaComponentType);
	FRPCContainer RPCs;

	TestFalse("Has queued RPCs", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, AnySchemaComponentType));

    return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_WHEN_one_value_has_been_added_THEN_it_is_queued)
{
	UObjectDummy* TargetObject = NewObject<UObjectDummy>();
	FUnrealObjectRef ObjecRef = GenerateObjectRef(TargetObject);
	FPendingRPCParamsPtr Params = CreateMockParameters(TargetObject, AnySchemaComponentType);
	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(Params), AnySchemaComponentType);

	TestTrue("Has queued RPCs", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, AnySchemaComponentType));

    return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_WHEN_multiple_values_of_same_type_have_been_added_THEN_they_are_queued)
{
	UObjectDummy* TargetObject = NewObject<UObjectDummy>();
	FUnrealObjectRef ObjecRef = GenerateObjectRef(TargetObject);
	FPendingRPCParamsPtr Params1 = CreateMockParameters(TargetObject, AnyOtherSchemaComponentType);
	FPendingRPCParamsPtr Params2 = CreateMockParameters(TargetObject, AnyOtherSchemaComponentType);
	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(Params1), AnyOtherSchemaComponentType);
	RPCs.QueueRPC(MoveTemp(Params2), AnyOtherSchemaComponentType);

	TestTrue("Has queued RPCs", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, AnyOtherSchemaComponentType));

    return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_storing_one_value_WHEN_processed_once_THEN_nothing_is_queued)
{
	UObjectDummy* TargetObject = NewObject<UObjectDummy>();
	FUnrealObjectRef ObjecRef = GenerateObjectRef(TargetObject);
	FPendingRPCParamsPtr Params = CreateMockParameters(TargetObject, AnySchemaComponentType);
	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(Params), AnySchemaComponentType);

	FProcessRPCDelegate Delegate;
	Delegate.BindUObject(TargetObject, &UObjectDummy::ProcessRPC);
	RPCs.ProcessRPCs(Delegate);

	TestFalse("Has queued RPCs", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, AnySchemaComponentType));

    return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_storinng_multiple_values_of_same_type_WHEN_processed_once_THEN_nothing_is_queued)
{
	UObjectDummy* TargetObject = NewObject<UObjectDummy>();
	FUnrealObjectRef ObjecRef = GenerateObjectRef(TargetObject);
	FPendingRPCParamsPtr Params1 = CreateMockParameters(TargetObject, AnyOtherSchemaComponentType);
	FPendingRPCParamsPtr Params2 = CreateMockParameters(TargetObject, AnyOtherSchemaComponentType);
	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(Params1), AnyOtherSchemaComponentType);
	RPCs.QueueRPC(MoveTemp(Params2), AnyOtherSchemaComponentType);

	FProcessRPCDelegate Delegate;
	Delegate.BindUObject(TargetObject, &UObjectDummy::ProcessRPC);
	RPCs.ProcessRPCs(Delegate);

	TestFalse("Has queued RPCs", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, AnyOtherSchemaComponentType));

    return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_WHEN_multiple_values_of_different_type_have_been_added_THEN_they_are_queued)
{
	UObjectDummy* TargetObject = NewObject<UObjectDummy>();

	FPendingRPCParamsPtr ParamsUnreliable = CreateMockParameters(TargetObject, AnyOtherSchemaComponentType);
	FUnrealObjectRef ObjecRefUnreliable = ParamsUnreliable->ObjectRef;

	FPendingRPCParamsPtr ParamsReliable = CreateMockParameters(TargetObject, AnySchemaComponentType);
	FUnrealObjectRef ObjecRefReliable = ParamsReliable->ObjectRef;

	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(ParamsUnreliable), AnyOtherSchemaComponentType);
	RPCs.QueueRPC(MoveTemp(ParamsReliable), AnySchemaComponentType);

	TestTrue("Has queued RPCs", RPCs.ObjectHasRPCsQueuedOfType(ObjecRefUnreliable.Entity, AnyOtherSchemaComponentType));
	TestTrue("Has queued RPCs", RPCs.ObjectHasRPCsQueuedOfType(ObjecRefReliable.Entity, AnySchemaComponentType));

    return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_storing_multiple_values_of_different_type_WHEN_processed_once_THEN_nothing_is_queued)
{
	UObjectDummy* TargetObject = NewObject<UObjectDummy>();
	FUnrealObjectRef ObjecRef = GenerateObjectRef(TargetObject);
	FPendingRPCParamsPtr ParamsUnreliable = CreateMockParameters(TargetObject, AnyOtherSchemaComponentType);
	FPendingRPCParamsPtr ParamsReliable = CreateMockParameters(TargetObject, AnySchemaComponentType);

	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(ParamsUnreliable), AnyOtherSchemaComponentType);
	RPCs.QueueRPC(MoveTemp(ParamsReliable), AnySchemaComponentType);

	FProcessRPCDelegate Delegate;
	Delegate.BindUObject(TargetObject, &UObjectDummy::ProcessRPC);
	RPCs.ProcessRPCs(Delegate);

	TestFalse("Has queued RPCs", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, AnyOtherSchemaComponentType));
	TestFalse("Has queued RPCs", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, AnySchemaComponentType));

    return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_storing_multiple_values_of_different_type_WHEN_processed_once_THEN_values_have_been_processed_in_order)
{
	UObjectSpy* TargetObject = NewObject<UObjectSpy>();
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
	Delegate.BindUObject(TargetObject, &UObjectSpy::ProcessRPC);
	RPCs.ProcessRPCs(Delegate);

	bool bProcessedInOrder = true;
	for (const auto& ProcessedIndicesOfType : TargetObject->ProcessedRPCIndices)
	{
		TArray<uint32>& storedIndicesOfType = RPCIndices.FindChecked(ProcessedIndicesOfType.Key);

		check(ProcessedIndicesOfType.Value.Num() == storedIndicesOfType.Num());
		const int32 numIndices = ProcessedIndicesOfType.Value.Num();

		for (int i = 0; i < numIndices; ++i)
		{
			if (ProcessedIndicesOfType.Value[i] != storedIndicesOfType[i])
			{
				bProcessedInOrder = false;
				break;
			}
		}
	}

	TestTrue("Queued RPCs have been processed in order", bProcessedInOrder);

    return true;
}
