// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "ObjectDummy.h"
#include "ObjectSpy.h"
#include "ObjectStub.h"

#include "Schema/RPCPayload.h"
#include "SpatialGDKSettings.h"
#include "Utils/RPCContainer.h"

#include "CoreMinimal.h"

#define RPCCONTAINER_TEST(TestName) GDK_TEST(Core, FRPCContainer, TestName)

using namespace SpatialGDK;

namespace
{
ERPCType AnySchemaComponentType = ERPCType::ClientReliable;
ERPCType AnyOtherSchemaComponentType = ERPCType::ClientUnreliable;

FUnrealObjectRef GenerateObjectRef(UObject* TargetObject)
{
	return FUnrealObjectRef{ Worker_EntityId(TargetObject), 0 };
}

uint32 GeneratePayloadFunctionIndex()
{
	static uint32 FreeIndex = 0;
	return FreeIndex++;
}

FPendingRPCParams CreateMockParameters(UObject* TargetObject, ERPCType Type)
{
	// Use PayloadData as a place to store RPC type
	RPCPayload Payload(0, GeneratePayloadFunctionIndex(), SpyUtils::RPCTypeToByteArray(Type));
	int ReliableRPCIndex = 0;

	FUnrealObjectRef ObjectRef = GenerateObjectRef(TargetObject);

	return FPendingRPCParams{ ObjectRef, Type, MoveTemp(Payload) };
}
} // anonymous namespace

RPCCONTAINER_TEST(GIVEN_a_container_WHEN_nothing_has_been_added_THEN_nothing_is_queued)
{
	UObjectDummy* TargetObject = NewObject<UObjectDummy>();
	FPendingRPCParams Params = CreateMockParameters(TargetObject, AnySchemaComponentType);
	FRPCContainer RPCs(ERPCQueueType::Send);

	TestFalse("Has queued RPCs", RPCs.ObjectHasRPCsQueuedOfType(Params.ObjectRef.Entity, AnySchemaComponentType));

	return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_WHEN_one_value_has_been_added_THEN_it_is_queued)
{
	UObjectStub* TargetObject = NewObject<UObjectStub>();
	FPendingRPCParams Params = CreateMockParameters(TargetObject, AnySchemaComponentType);
	FRPCContainer RPCs(ERPCQueueType::Send);
	RPCs.BindProcessingFunction(FProcessRPCDelegate::CreateUObject(TargetObject, &UObjectStub::ProcessRPC));

	RPCs.ProcessOrQueueRPC(Params.ObjectRef, Params.Type, MoveTemp(Params.Payload));

	TestTrue("Has queued RPCs", RPCs.ObjectHasRPCsQueuedOfType(Params.ObjectRef.Entity, AnySchemaComponentType));

	return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_WHEN_multiple_values_of_same_type_have_been_added_THEN_they_are_queued)
{
	UObjectStub* TargetObject = NewObject<UObjectStub>();
	FPendingRPCParams Params1 = CreateMockParameters(TargetObject, AnyOtherSchemaComponentType);
	FPendingRPCParams Params2 = CreateMockParameters(TargetObject, AnyOtherSchemaComponentType);
	FRPCContainer RPCs(ERPCQueueType::Send);
	RPCs.BindProcessingFunction(FProcessRPCDelegate::CreateUObject(TargetObject, &UObjectStub::ProcessRPC));

	RPCs.ProcessOrQueueRPC(Params1.ObjectRef, Params1.Type, MoveTemp(Params1.Payload));
	RPCs.ProcessOrQueueRPC(Params2.ObjectRef, Params2.Type, MoveTemp(Params2.Payload));

	TestTrue("Has queued RPCs", RPCs.ObjectHasRPCsQueuedOfType(Params1.ObjectRef.Entity, AnyOtherSchemaComponentType));

	return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_storing_one_value_WHEN_processed_once_THEN_nothing_is_queued)
{
	UObjectDummy* TargetObject = NewObject<UObjectDummy>();
	FPendingRPCParams Params = CreateMockParameters(TargetObject, AnySchemaComponentType);
	FRPCContainer RPCs(ERPCQueueType::Send);
	RPCs.BindProcessingFunction(FProcessRPCDelegate::CreateUObject(TargetObject, &UObjectDummy::ProcessRPC));

	RPCs.ProcessOrQueueRPC(Params.ObjectRef, Params.Type, MoveTemp(Params.Payload));

	TestFalse("Has queued RPCs", RPCs.ObjectHasRPCsQueuedOfType(Params.ObjectRef.Entity, AnySchemaComponentType));

	return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_storing_multiple_values_of_same_type_WHEN_processed_once_THEN_nothing_is_queued)
{
	UObjectDummy* TargetObject = NewObject<UObjectDummy>();
	FPendingRPCParams Params1 = CreateMockParameters(TargetObject, AnyOtherSchemaComponentType);
	FPendingRPCParams Params2 = CreateMockParameters(TargetObject, AnyOtherSchemaComponentType);
	FRPCContainer RPCs(ERPCQueueType::Send);
	RPCs.BindProcessingFunction(FProcessRPCDelegate::CreateUObject(TargetObject, &UObjectDummy::ProcessRPC));

	RPCs.ProcessOrQueueRPC(Params1.ObjectRef, Params1.Type, MoveTemp(Params1.Payload));
	RPCs.ProcessOrQueueRPC(Params2.ObjectRef, Params2.Type, MoveTemp(Params2.Payload));

	TestFalse("Has queued RPCs", RPCs.ObjectHasRPCsQueuedOfType(Params1.ObjectRef.Entity, AnyOtherSchemaComponentType));

	return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_WHEN_multiple_values_of_different_type_have_been_added_THEN_they_are_queued)
{
	UObjectStub* TargetObject = NewObject<UObjectStub>();

	FPendingRPCParams ParamsUnreliable = CreateMockParameters(TargetObject, AnyOtherSchemaComponentType);
	FPendingRPCParams ParamsReliable = CreateMockParameters(TargetObject, AnySchemaComponentType);

	FRPCContainer RPCs(ERPCQueueType::Send);
	RPCs.BindProcessingFunction(FProcessRPCDelegate::CreateUObject(TargetObject, &UObjectStub::ProcessRPC));

	RPCs.ProcessOrQueueRPC(ParamsUnreliable.ObjectRef, ParamsUnreliable.Type, MoveTemp(ParamsUnreliable.Payload));
	RPCs.ProcessOrQueueRPC(ParamsReliable.ObjectRef, ParamsReliable.Type, MoveTemp(ParamsReliable.Payload));

	TestTrue("Has queued RPCs", RPCs.ObjectHasRPCsQueuedOfType(ParamsUnreliable.ObjectRef.Entity, AnyOtherSchemaComponentType));
	TestTrue("Has queued RPCs", RPCs.ObjectHasRPCsQueuedOfType(ParamsReliable.ObjectRef.Entity, AnySchemaComponentType));

	return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_storing_multiple_values_of_different_type_WHEN_processed_once_THEN_nothing_is_queued)
{
	UObjectDummy* TargetObject = NewObject<UObjectDummy>();
	FPendingRPCParams ParamsUnreliable = CreateMockParameters(TargetObject, AnyOtherSchemaComponentType);
	FPendingRPCParams ParamsReliable = CreateMockParameters(TargetObject, AnySchemaComponentType);

	FRPCContainer RPCs(ERPCQueueType::Send);
	RPCs.BindProcessingFunction(FProcessRPCDelegate::CreateUObject(TargetObject, &UObjectDummy::ProcessRPC));

	RPCs.ProcessOrQueueRPC(ParamsUnreliable.ObjectRef, ParamsUnreliable.Type, MoveTemp(ParamsUnreliable.Payload));
	RPCs.ProcessOrQueueRPC(ParamsReliable.ObjectRef, ParamsReliable.Type, MoveTemp(ParamsReliable.Payload));

	TestFalse("Has queued RPCs", RPCs.ObjectHasRPCsQueuedOfType(ParamsUnreliable.ObjectRef.Entity, AnyOtherSchemaComponentType));
	TestFalse("Has queued RPCs", RPCs.ObjectHasRPCsQueuedOfType(ParamsReliable.ObjectRef.Entity, AnySchemaComponentType));

	return true;
}

RPCCONTAINER_TEST(GIVEN_a_container_storing_multiple_values_of_different_type_WHEN_processed_once_THEN_values_have_been_processed_in_order)
{
	UObjectSpy* TargetObject = NewObject<UObjectSpy>();
	FUnrealObjectRef ObjectRef = GenerateObjectRef(TargetObject);
	FRPCContainer RPCs(ERPCQueueType::Send);
	RPCs.BindProcessingFunction(FProcessRPCDelegate::CreateUObject(TargetObject, &UObjectSpy::ProcessRPC));

	TMap<ERPCType, TArray<uint32>> RPCIndices;

	for (int i = 0; i < 4; ++i)
	{
		FPendingRPCParams ParamsUnreliable = CreateMockParameters(TargetObject, AnyOtherSchemaComponentType);
		FPendingRPCParams ParamsReliable = CreateMockParameters(TargetObject, AnySchemaComponentType);

		RPCIndices.FindOrAdd(AnyOtherSchemaComponentType).Push(ParamsUnreliable.Payload.Index);
		RPCIndices.FindOrAdd(AnySchemaComponentType).Push(ParamsReliable.Payload.Index);

		RPCs.ProcessOrQueueRPC(ObjectRef, ParamsUnreliable.Type, MoveTemp(ParamsUnreliable.Payload));
		RPCs.ProcessOrQueueRPC(ObjectRef, ParamsReliable.Type, MoveTemp(ParamsReliable.Payload));
	}

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

RPCCONTAINER_TEST(GIVEN_a_container_with_one_value_WHEN_processing_after_RPCQueueWarningDefaultTimeout_seconds_THEN_warning_is_logged)
{
	UObjectStub* TargetObject = NewObject<UObjectStub>();
	FPendingRPCParams Params = CreateMockParameters(TargetObject, AnySchemaComponentType);
	FRPCContainer RPCs(ERPCQueueType::Send);
	RPCs.BindProcessingFunction(FProcessRPCDelegate::CreateUObject(TargetObject, &UObjectStub::ProcessRPC));
	RPCs.ProcessOrQueueRPC(Params.ObjectRef, Params.Type, MoveTemp(Params.Payload));

	AddExpectedError(TEXT("Unresolved Parameters"), EAutomationExpectedErrorFlags::Contains, 1);

	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	check(SpatialGDKSettings != nullptr);
	FPlatformProcess::Sleep(SpatialGDKSettings->RPCQueueWarningDefaultTimeout);
	RPCs.ProcessRPCs();

	return true;
}
