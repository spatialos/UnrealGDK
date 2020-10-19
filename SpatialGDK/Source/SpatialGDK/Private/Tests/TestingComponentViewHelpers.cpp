// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestingComponentViewHelpers.h"

#include "CoreMinimal.h"

void TestingComponentViewHelpers::AddEntityComponentToStaticComponentView(USpatialStaticComponentView& StaticComponentView,
																		  const Worker_EntityId EntityId,
																		  const Worker_ComponentId ComponentId,
																		  Schema_ComponentData* ComponentData,
																		  const Worker_Authority Authority)
{
	Worker_AddComponentOp AddComponentOp;
	AddComponentOp.entity_id = EntityId;
	AddComponentOp.data.component_id = ComponentId;
	AddComponentOp.data.schema_type = ComponentData;
	StaticComponentView.OnAddComponent(AddComponentOp);

	Worker_AuthorityChangeOp AuthorityChangeOp;
	AuthorityChangeOp.entity_id = EntityId;
	AuthorityChangeOp.component_id = ComponentId;
	AuthorityChangeOp.authority = Authority;
	StaticComponentView.OnAuthorityChange(AuthorityChangeOp);
}

void TestingComponentViewHelpers::AddEntityComponentToStaticComponentView(USpatialStaticComponentView& StaticComponentView,
																		  const Worker_EntityId EntityId,
																		  const Worker_ComponentId ComponentId,
																		  const Worker_Authority Authority)
{
	AddEntityComponentToStaticComponentView(StaticComponentView, EntityId, ComponentId, Schema_CreateComponentData(), Authority);
}
