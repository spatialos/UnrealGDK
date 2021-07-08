// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/WellKnownEntitySystem.h"

#include "Interop/SpatialReceiver.h"

DEFINE_LOG_CATEGORY(LogWellKnownEntitySystem);

namespace SpatialGDK
{
WellKnownEntitySystem::WellKnownEntitySystem(const FSubView& SubView, USpatialWorkerConnection* InConnection,
											 USpatialNetDriver* InNetDriver, const int InNumberOfWorkers,
											 SpatialVirtualWorkerTranslator& InVirtualWorkerTranslator,
											 UGlobalStateManager& InGlobalStateManager)
	: SubView(&SubView)
	, VirtualWorkerTranslator(&InVirtualWorkerTranslator)
	, GlobalStateManager(&InGlobalStateManager)
	, Connection(InConnection)
	, NetDriver(InNetDriver)
	, NumberOfWorkers(InNumberOfWorkers)
{
}

void WellKnownEntitySystem::Advance()
{
	const FSubViewDelta& SubViewDelta = SubView->GetViewDelta();
	for (const EntityDelta& Delta : SubViewDelta.EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			for (const ComponentChange& Change : Delta.ComponentUpdates)
			{
				ProcessComponentUpdate(Change.ComponentId, Change.Update);
			}
			for (const AuthorityChange& Change : Delta.AuthorityGained)
			{
				ProcessAuthorityGain(Delta.EntityId, Change.ComponentSetId);
			}
			break;
		}
		case EntityDelta::ADD:
			ProcessEntityAdd(Delta.EntityId);
			break;
		default:
			break;
		}
	}

	if (VirtualWorkerTranslationManager.IsValid())
	{
		VirtualWorkerTranslationManager->Advance(*SubView->GetViewDelta().WorkerMessages);
	}
}

void WellKnownEntitySystem::ProcessComponentUpdate(const Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update)
{
	switch (ComponentId)
	{
	case SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID:
#if WITH_EDITOR
		GlobalStateManager->OnShutdownComponentUpdate(Update);
#endif // WITH_EDITOR
		break;
	default:
		break;
	}
}

void WellKnownEntitySystem::ProcessAuthorityGain(const Worker_EntityId EntityId, const Worker_ComponentSetId ComponentSetId)
{
	if (SubView->GetView()[EntityId].Components.ContainsByPredicate(
			SpatialGDK::ComponentIdEquality{ SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID }))
	{
		InitializeVirtualWorkerTranslationManager();
	}
}

void WellKnownEntitySystem::ProcessEntityAdd(const Worker_EntityId EntityId)
{
	const EntityViewElement& Element = SubView->GetView()[EntityId];
	for (const Worker_ComponentSetId ComponentId : Element.Authority)
	{
		ProcessAuthorityGain(EntityId, ComponentId);
	}
}

// This is only called if this worker has been selected by SpatialOS to be authoritative
// for the TranslationManager, otherwise the manager will never be instantiated.
void WellKnownEntitySystem::InitializeVirtualWorkerTranslationManager()
{
	VirtualWorkerTranslationManager = MakeUnique<SpatialVirtualWorkerTranslationManager>(Connection, NetDriver, VirtualWorkerTranslator);
	VirtualWorkerTranslationManager->SetNumberOfVirtualWorkers(NumberOfWorkers);
}

} // Namespace SpatialGDK
