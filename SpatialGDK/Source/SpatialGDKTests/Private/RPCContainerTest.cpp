// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

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
	static ESchemaComponentType ServerReliableType = ESchemaComponentType::SCHEMA_ServerReliableRPC;
	static ESchemaComponentType ServerUnreliableType = ESchemaComponentType::SCHEMA_ServerUnreliableRPC;
	static ESchemaComponentType MulticastType = ESchemaComponentType::SCHEMA_NetMulticastRPC;
	static ESchemaComponentType CrossServerType = ESchemaComponentType::SCHEMA_CrossServerRPC;

	static FUnrealObjectRef GenerateObjectRef(UObject* TargetObject)
	{
		return FUnrealObjectRef{ Worker_EntityId(TargetObject), 0 };
	}

	static uint32 GeneratePayloadFunctionIndex()
	{
		static uint32 freeIndex = 0;
		return freeIndex++;
	}

	static FPendingRPCParamsPtr CreateMockParameters(UObject* TargetObject, ESchemaComponentType Type)
	{
		// Use PayloadData as a place to store RPC type
		RPCPayload Payload(0, GeneratePayloadFunctionIndex(), TypeToArray(Type));
		int ReliableRPCIndex = 0;

		FUnrealObjectRef ObjectRef = GenerateObjectRef(TargetObject);

		return MakeUnique<FPendingRPCParams>(ObjectRef, MoveTemp(Payload), ReliableRPCIndex);
	}
}

TEST(FRPCContainer, GIVEN_a_container_WHEN_nothing_has_been_added_THEN_nothing_is_queued)
{
	UMockObject* TargetObject = NewObject<UMockObject>();
	FUnrealObjectRef ObjecRef = GenerateObjectRef(TargetObject);
	FPendingRPCParamsPtr Params = CreateMockParameters(TargetObject, ReliableType);
	FRPCContainer RPCs;

	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, ReliableType));

    return true;
}

TEST(FRPCContainer, GIVEN_a_container_WHEN_one_value_has_been_added_THEN_it_is_queued)
{
	UMockObject* TargetObject = NewObject<UMockObject>();
	FUnrealObjectRef ObjecRef = GenerateObjectRef(TargetObject);
	FPendingRPCParamsPtr Params = CreateMockParameters(TargetObject, ReliableType);
	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(Params), ReliableType);

	TestTrue("Queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, ReliableType));

    return true;
}

TEST(FRPCContainer, GIVEN_a_container_WHEN_two_values_of_same_type_have_been_added_THEN_they_are_queued)
{
	UMockObject* TargetObject = NewObject<UMockObject>();
	FUnrealObjectRef ObjecRef = GenerateObjectRef(TargetObject);
	FPendingRPCParamsPtr Params1 = CreateMockParameters(TargetObject, UnreliableType);
	FPendingRPCParamsPtr Params2 = CreateMockParameters(TargetObject, UnreliableType);
	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(Params1), UnreliableType);
	RPCs.QueueRPC(MoveTemp(Params2), UnreliableType);

	TestTrue("Queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, UnreliableType));

    return true;
}

TEST(FRPCContainer, GIVEN_a_container_WHEN_one_value_has_been_added_and_processed_THEN_nothing_is_queued)
{
	UMockObject* TargetObject = NewObject<UMockObject>();
	FUnrealObjectRef ObjecRef = GenerateObjectRef(TargetObject);
	FPendingRPCParamsPtr Params = CreateMockParameters(TargetObject, ReliableType);
	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(Params), ReliableType);

	FProcessRPCDelegate Delegate;
	Delegate.BindUObject(TargetObject, &UMockObject::ProcessRPC);
	RPCs.ProcessRPCs(Delegate);

	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, ReliableType));

    return true;
}

TEST(FRPCContainer, GIVEN_a_container_WHEN_two_values_of_same_type_have_been_added_and_processed_THEN_nothing_is_queued)
{
	UMockObject* TargetObject = NewObject<UMockObject>();
	FUnrealObjectRef ObjecRef = GenerateObjectRef(TargetObject);
	FPendingRPCParamsPtr Params1 = CreateMockParameters(TargetObject, UnreliableType);
	FPendingRPCParamsPtr Params2 = CreateMockParameters(TargetObject, UnreliableType);
	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(Params1), UnreliableType);
	RPCs.QueueRPC(MoveTemp(Params2), UnreliableType);

	FProcessRPCDelegate Delegate;
	Delegate.BindUObject(TargetObject, &UMockObject::ProcessRPC);
	RPCs.ProcessRPCs(Delegate);

	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, UnreliableType));

    return true;
}

TEST(FRPCContainer, GIVEN_a_container_WHEN_two_values_of_different_type_have_been_added_THEN_they_are_queued)
{
	UMockObject* TargetObject = NewObject<UMockObject>();

	FPendingRPCParamsPtr ParamsUnreliable = CreateMockParameters(TargetObject, UnreliableType);
	FUnrealObjectRef ObjecRefUnreliable = ParamsUnreliable->ObjectRef;

	FPendingRPCParamsPtr ParamsReliable = CreateMockParameters(TargetObject, ReliableType);
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
	FUnrealObjectRef ObjecRef = GenerateObjectRef(TargetObject);
	FPendingRPCParamsPtr ParamsUnreliable = CreateMockParameters(TargetObject, UnreliableType);
	FPendingRPCParamsPtr ParamsReliable = CreateMockParameters(TargetObject, ReliableType);

	FRPCContainer RPCs;

	RPCs.QueueRPC(MoveTemp(ParamsUnreliable), UnreliableType);
	RPCs.QueueRPC(MoveTemp(ParamsReliable), ReliableType);

	FProcessRPCDelegate Delegate;
	Delegate.BindUObject(TargetObject, &UMockObject::ProcessRPC);
	RPCs.ProcessRPCs(Delegate);

	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, UnreliableType));
	TestFalse("No queued RPCs of such type", RPCs.ObjectHasRPCsQueuedOfType(ObjecRef.Entity, ReliableType));

    return true;
}

TEST(FRPCContainer, GIVEN_a_container_WHEN_six_values_of_different_type_have_been_added_and_processed_THEN_they_have_been_processed_in_order)
{
	UMockObject* TargetObject = NewObject<UMockObject>();
	FUnrealObjectRef ObjectRef = GenerateObjectRef(TargetObject);
	FRPCContainer RPCs;

	TMap<ESchemaComponentType, TArray<uint32>> RPCIndices;

	for (int i = 0; i < 4; ++i)
	{
		FPendingRPCParamsPtr ParamsUnreliable = CreateMockParameters(TargetObject, UnreliableType);
		FPendingRPCParamsPtr ParamsReliable = CreateMockParameters(TargetObject, ReliableType);
		FPendingRPCParamsPtr ParamsCrossServer = CreateMockParameters(TargetObject, CrossServerType);
		FPendingRPCParamsPtr ParamsMulticast = CreateMockParameters(TargetObject, MulticastType);

		RPCIndices.FindOrAdd(UnreliableType).Push(ParamsUnreliable->Payload.Index);
		RPCIndices.FindOrAdd(ReliableType).Push(ParamsReliable->Payload.Index);
		RPCIndices.FindOrAdd(CrossServerType).Push(ParamsCrossServer->Payload.Index);
		RPCIndices.FindOrAdd(MulticastType).Push(ParamsMulticast->Payload.Index);

		RPCs.QueueRPC(MoveTemp(ParamsUnreliable), UnreliableType);
		RPCs.QueueRPC(MoveTemp(ParamsReliable), ReliableType);
		RPCs.QueueRPC(MoveTemp(ParamsCrossServer), CrossServerType);
		RPCs.QueueRPC(MoveTemp(ParamsMulticast), MulticastType);
	}

	FProcessRPCDelegate Delegate;
	Delegate.BindUObject(TargetObject, &UMockObject::ProcessRPC);
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
