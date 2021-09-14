// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/ChangeInterest.h"

DEFINE_LOG_CATEGORY(LogChangeInterest);

namespace SpatialGDK
{
void ChangeInterestQuery::DebugOutput(const FString& DiffType) const
{
	// Minimal output
	if (ComponentSets.Find(SpatialConstants::OWNER_ONLY_COMPONENT_SET_ID) != INDEX_NONE)
	{
		UE_LOG(LogChangeInterest, Log, TEXT("Interest diff: client auth interest"));
	}
	else
	{
		UE_LOG(LogChangeInterest, Log, TEXT("Interest diff: client non auth interest"));
	}

	const FString ComponentsString = FString::JoinBy(Components, TEXT(" "), [](const Worker_ComponentId ComponentId) {
		return FString::Printf(TEXT("%d"), ComponentId);
	});
	UE_LOG(LogChangeInterest, Verbose, TEXT("Interest diff: components %s"), *ComponentsString);

	const FString ComponentSetsString = FString::JoinBy(ComponentSets, TEXT(" "), [](const Worker_ComponentSetId ComponentSetId) {
		return FString::Printf(TEXT("%d"), ComponentSetId);
	});
	UE_LOG(LogChangeInterest, Verbose, TEXT("Interest diff: component sets %s"), *ComponentSetsString);

	const FString EntitiesString = FString::JoinBy(Entities, TEXT(" "), [](const Worker_EntityId_Key EntityId) {
		return FString::Printf(TEXT("%lld"), EntityId);
	});

	UE_LOG(LogChangeInterest, Log, TEXT("Interest diff: %s %s"), *DiffType, *EntitiesString);
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
	UE_LOG(LogChangeInterest, Log, TEXT("Interest diff: client entity id %lld"), SystemEntityId);

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

Worker_CommandRequest ChangeInterestRequest::CreateRequest() const
{
	Worker_CommandRequest Request{};
	Request.component_id = SpatialConstants::WORKER_COMPONENT_ID;
	Request.command_index = SpatialConstants::WORKER_CHANGE_INTEREST_COMMAND_ID;
	Request.schema_type = Schema_CreateCommandRequest();

	Schema_Object* RequestObject = Schema_GetCommandRequestObject(Request.schema_type);

	Schema_AddEntityId(RequestObject, 1, SystemEntityId);

	if (QueriesToAdd.Num() > 0)
	{
		for (const ChangeInterestQuery& Query : QueriesToAdd)
		{
			Schema_Object* QueriesToAddObject = Schema_AddObject(RequestObject, 2);

			Schema_Object* ResultTypeObject = Schema_AddObject(QueriesToAddObject, 1);
			Schema_AddEntityIdList(QueriesToAddObject, 2, (const Worker_EntityId*)Query.Entities.GetData(), Query.Entities.Num());

			Schema_AddUint32List(ResultTypeObject, 1, Query.Components.GetData(), Query.Components.Num());
			Schema_AddUint32List(ResultTypeObject, 2, Query.ComponentSets.GetData(), Query.ComponentSets.Num());
		}
	}

	if (QueriesToRemove.Num() > 0)
	{
		for (const ChangeInterestQuery& Query : QueriesToRemove)
		{
			Schema_Object* QueriesToRemoveObject = Schema_AddObject(RequestObject, 3);

			Schema_Object* ResultTypeObject = Schema_AddObject(QueriesToRemoveObject, 1);
			Schema_AddEntityIdList(QueriesToRemoveObject, 2, (const Worker_EntityId*)Query.Entities.GetData(), Query.Entities.Num());

			Schema_AddUint32List(ResultTypeObject, 1, Query.Components.GetData(), Query.Components.Num());
			Schema_AddUint32List(ResultTypeObject, 2, Query.ComponentSets.GetData(), Query.ComponentSets.Num());
		}
	}

	Schema_AddBool(RequestObject, 4, bOverwrite);
	return Request;
}

} // namespace SpatialGDK
