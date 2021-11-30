// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/ChangeInterest.h"
#include "SpatialView/SpatialOSWorker.h"

DEFINE_LOG_CATEGORY(LogChangeInterest);

namespace SpatialGDK
{
inline void AddEntityListToSchema(Schema_Object* Object, Schema_FieldId Id, TArray<Worker_EntityId_Key> Entities)
{
	uint8* PayloadBuffer = Schema_AllocateBuffer(Object, sizeof(Worker_EntityId_Key) * Entities.Num());
	FMemory::Memcpy(PayloadBuffer, Entities.GetData(), sizeof(Worker_EntityId_Key) * Entities.Num());
	Schema_AddEntityIdList(Object, Id, (Worker_EntityId*)PayloadBuffer, Entities.Num());
}

inline void AddUint32ListToSchema(Schema_Object* Object, Schema_FieldId Id, TArray<uint32> IntList)
{
	uint8* PayloadBuffer = Schema_AllocateBuffer(Object, sizeof(uint32) * IntList.Num());
	FMemory::Memcpy(PayloadBuffer, IntList.GetData(), sizeof(uint32) * IntList.Num());
	Schema_AddUint32List(Object, Id, (uint32*)PayloadBuffer, IntList.Num());
}

void ChangeInterestQuery::DebugOutput(const FString& DiffType) const
{
	const FString EntitiesString = FString::JoinBy(Entities, TEXT(" "), [](const Worker_EntityId_Key EntityId) {
		return FString::Printf(TEXT("%lld"), EntityId);
	});
	UE_LOG(LogChangeInterest, Log, TEXT("Interest diff: %s %s"), *DiffType, *EntitiesString);

	const FString ComponentsString = FString::JoinBy(ResultComponentIds, TEXT(" "), [](const Worker_ComponentId ComponentId) {
		return FString::Printf(TEXT("%d"), ComponentId);
	});
	UE_LOG(LogChangeInterest, Log, TEXT("Interest diff: components %s"), *ComponentsString);

	const FString ComponentSetsString = FString::JoinBy(ResultComponentSetIds, TEXT(" "), [](const Worker_ComponentSetId ComponentSetId) {
		return FString::Printf(TEXT("%d"), ComponentSetId);
	});
	UE_LOG(LogChangeInterest, Log, TEXT("Interest diff: component sets %s"), *ComponentSetsString);
}

void ChangeInterestRequest::Clear()
{
	SystemEntityId = SpatialConstants::INVALID_ENTITY_ID;
	QueriesToAdd.Empty();
	QueriesToRemove.Empty();
	bOverwrite = false;
}

void ChangeInterestRequest::DebugOutput() const
{
	UE_LOG(LogChangeInterest, Log, TEXT("Interest diff: system entity id %lld"), SystemEntityId);

	for (const auto& Query : QueriesToAdd)
	{
		Query.DebugOutput(TEXT("ADD"));
	}

	for (const auto& Query : QueriesToRemove)
	{
		Query.DebugOutput(TEXT("REMOVE"));
	}

	UE_LOG(LogChangeInterest, Log, TEXT("Interest diff: overwrite %d"), bOverwrite);
}

inline void AddEntityListToSchema(Schema_Object* Object, Schema_FieldId Id, TArrayView<const Worker_EntityId> Entities)
{
	uint8* PayloadBuffer = Schema_AllocateBuffer(Object, sizeof(Worker_EntityId) * Entities.Num());
	FMemory::Memcpy(PayloadBuffer, Entities.GetData(), sizeof(Worker_EntityId) * Entities.Num());
	Schema_AddEntityIdList(Object, Id, (const Worker_EntityId*)PayloadBuffer, Entities.Num());
}

inline void AddUint32ListToSchema(Schema_Object* Object, Schema_FieldId Id, TArrayView<const uint32> IntList)
{
	uint8* PayloadBuffer = Schema_AllocateBuffer(Object, sizeof(uint32) * IntList.Num());
	FMemory::Memcpy(PayloadBuffer, IntList.GetData(), sizeof(uint32) * IntList.Num());
	Schema_AddUint32List(Object, Id, (const uint32*)PayloadBuffer, IntList.Num());
}

Worker_CommandRequest ChangeInterestRequest::CreateRequest() const
{
	Worker_CommandRequest Request{};
	Request.component_id = SpatialConstants::WORKER_COMPONENT_ID;
	Request.command_index = SpatialConstants::WORKER_CHANGE_INTEREST_COMMAND_ID;
	Request.schema_type = Schema_CreateCommandRequest();

	Schema_Object* RequestObject = Schema_GetCommandRequestObject(Request.schema_type);

	for (const ChangeInterestQuery& Query : QueriesToAdd)
	{
		Schema_Object* QueriesToAddObject = Schema_AddObject(RequestObject, 1);

		Schema_Object* ResultTypeObject = Schema_AddObject(QueriesToAddObject, 1);

		AddEntityListToSchema(QueriesToAddObject, 2, Query.Entities);
		Schema_AddBool(QueriesToAddObject, 3, Query.TrueConstraint);
		AddUint32ListToSchema(ResultTypeObject, 1, Query.ResultComponentIds);
		AddUint32ListToSchema(ResultTypeObject, 2, Query.ResultComponentSetIds);
	}

	for (const ChangeInterestQuery& Query : QueriesToRemove)
	{
		Schema_Object* QueriesToRemoveObject = Schema_AddObject(RequestObject, 2);

		Schema_Object* ResultTypeObject = Schema_AddObject(QueriesToRemoveObject, 1);

		AddEntityListToSchema(QueriesToRemoveObject, 2, Query.Entities);
		Schema_AddBool(QueriesToRemoveObject, 3, Query.TrueConstraint);
		AddUint32ListToSchema(ResultTypeObject, 1, Query.ResultComponentIds);
		AddUint32ListToSchema(ResultTypeObject, 2, Query.ResultComponentSetIds);
	}

	Schema_AddBool(RequestObject, 3, bOverwrite);
	return Request;
}

Worker_RequestId ChangeInterestRequest::SendRequest(ISpatialOSWorker& Connection)
{
	Worker_CommandRequest InterestRequestData = CreateRequest();
	CommandRequest InterestRequest(OwningCommandRequestPtr(InterestRequestData.schema_type), InterestRequestData.component_id,
								   InterestRequestData.command_index);
	return Connection.SendEntityCommandRequest(SystemEntityId, MoveTemp(InterestRequest));
}

} // namespace SpatialGDK
