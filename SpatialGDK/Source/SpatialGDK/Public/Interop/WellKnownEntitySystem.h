// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Connection/SpatialWorkerConnection.h"
#include "EngineClasses/SpatialVirtualWorkerTranslationManager.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "GlobalStateManager.h"
#include "SpatialView/SubView.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWellKnownEntitySystem, Log, All)

namespace SpatialGDK
{
class WellKnownEntitySystem
{
public:
	WellKnownEntitySystem(const FSubView& SubView, USpatialWorkerConnection* InConnection, USpatialNetDriver* InNetDriver,
						  int InNumberOfWorkers, SpatialVirtualWorkerTranslator& InVirtualWorkerTranslator,
						  UGlobalStateManager& InGlobalStateManager);
	void Advance();

private:
	void ProcessComponentUpdate(const Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update);
	void ProcessAuthorityGain(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId);
	void ProcessEntityAdd(const Worker_EntityId EntityId);

	void InitializeVirtualWorkerTranslationManager();

	const FSubView* SubView;

	TUniquePtr<SpatialVirtualWorkerTranslationManager> VirtualWorkerTranslationManager;
	SpatialVirtualWorkerTranslator* VirtualWorkerTranslator;
	UGlobalStateManager* GlobalStateManager;
	USpatialWorkerConnection* Connection;
	USpatialNetDriver* NetDriver;
	int NumberOfWorkers;
};

} // namespace SpatialGDK
