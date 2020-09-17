// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialStaticComponentView.h"
#include "SpatialCommonTypes.h"

#include "SpatialView/ComponentUpdate.h"
#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/SubView.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialLoadBalanceEnforcer, Log, All)

class SpatialVirtualWorkerTranslator;

class SPATIALGDK_API SpatialLoadBalanceEnforcer
{
public:
	SpatialLoadBalanceEnforcer(const PhysicalWorkerName& InWorkerId, const USpatialStaticComponentView* InStaticComponentView,
							   const SpatialGDK::FSubView& InSubView, const SpatialVirtualWorkerTranslator* InVirtualWorkerTranslator);

	static bool HandlesComponent(Worker_ComponentId ComponentId);

	void Advance();
	TArray<SpatialGDK::EntityComponentUpdate> GetAndClearAclUpdates();

	void MaybeCreateAclUpdate(const Worker_EntityId EntityId);

	void TagQuery(SpatialGDK::Query& QueryToTag) const;
	void TagEntity(TArray<FWorkerComponentData>& Components) const;

private:
	void CreateAclUpdate(const Worker_EntityId EntityId);

	const PhysicalWorkerName WorkerId;
	// todo: come back to removing
	TWeakObjectPtr<const USpatialStaticComponentView> StaticComponentView;
	const SpatialGDK::FSubView* SubView;
	const SpatialVirtualWorkerTranslator* VirtualWorkerTranslator;

	TArray<SpatialGDK::EntityComponentUpdate> AclUpdates;
};
