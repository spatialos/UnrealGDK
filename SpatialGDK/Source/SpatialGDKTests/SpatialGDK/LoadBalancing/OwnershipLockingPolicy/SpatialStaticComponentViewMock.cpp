// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialStaticComponentViewMock.h"

#include "Schema/AuthorityIntent.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "WorkerSDK/improbable/c_worker.h"

void USpatialStaticComponentViewMock::Init(Worker_EntityId EntityId, Worker_Authority Authority, VirtualWorkerId VirtWorkerId)
{
	Worker_AddComponentOp AddCompOp;
	AddCompOp.entity_id = EntityId;
	AddCompOp.data = SpatialGDK::AuthorityIntent::CreateAuthorityIntentData(VirtWorkerId);

	OnAddComponent(AddCompOp);

	Worker_AuthorityChangeOp AuthChangeOp;
	AuthChangeOp.entity_id = EntityId;
	AuthChangeOp.component_id = SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID;
	AuthChangeOp.authority = Authority;
	OnAuthorityChange(AuthChangeOp);
}
