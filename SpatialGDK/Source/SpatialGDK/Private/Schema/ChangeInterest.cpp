// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/ChangeInterest.h"

DEFINE_LOG_CATEGORY(LogChangeInterest);

namespace SpatialGDK
{
void ChangeInterestQuery::DebugOutput(const FString& DiffType) const
{
	// Minimal output
	if (Components.Find(SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID) != INDEX_NONE)
	{
		UE_LOG(LogChangeInterest, Log, TEXT("Interest diff: client auth interest"));
	}
	else
	{
		UE_LOG(LogChangeInterest, Log, TEXT("Interest diff: client non auth interest"));
	}

	/*
	// Verbose output
	FString ComponentsString;
	for (const auto& Component : Components)
	{
		ComponentsString += FString::Format(TEXT("{0} "), { Component });
	}

	UE_LOG(LogChangeInterest, Log, TEXT("Interest diff: components %s"), *ComponentsString);

	FString ComponentSetsString;
	for (const auto& ComponentSet : ComponentSets)
	{
		ComponentSetsString += FString::Format(TEXT("{0} "), { ComponentSet });
	}

	UE_LOG(LogChangeInterest, Log, TEXT("Interest diff: component sets %s"), *ComponentSetsString);
	*/

	FString EntitiesString;
	for (const auto& EntityId : Entities)
	{
		EntitiesString += FString::Format(TEXT("{0} "), { EntityId });
	}

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

void ChangeInterestRequest::CreateRequest(Worker_CommandRequest& OutCommandRequest) const
{
	OutCommandRequest.component_id = SpatialConstants::WORKER_COMPONENT_ID;
	OutCommandRequest.command_index = SpatialConstants::WORKER_CHANGE_INTEREST_COMMAND_ID;
	OutCommandRequest.schema_type = Schema_CreateCommandRequest();

	Schema_Object* RequestObject = Schema_GetCommandRequestObject(OutCommandRequest.schema_type);

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
}

} // namespace SpatialGDK
