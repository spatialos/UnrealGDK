// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/ServerCrashHandler.h"

#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Improbable/SpatialEngineConstants.h"
#include "Interop/SpatialReceiver.h"
#include "Schema/PlayerControllerServer.h"
#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/EntityDelta.h"
#include "SpatialView/SubView.h"

DEFINE_LOG_CATEGORY(LogServerCrashHandler);

namespace SpatialGDK
{
ServerCrashHandler::ServerCrashHandler(const FSubView& InWellKnownSubView, const FSubView& InPlayerControllerSubView,
									   USpatialNetDriver* InNetDriver)
	: WellKnownSubView(&InWellKnownSubView)
	, PlayerControllerSubView(&InPlayerControllerSubView)
	, NetDriver(InNetDriver)
	, AuthServerWorkerEntityId(SpatialConstants::INVALID_ENTITY_ID)
{
}

void ServerCrashHandler::Advance()
{
	const FSubViewDelta& PlayerControllerSubViewDelta = PlayerControllerSubView->GetViewDelta();
	for (const EntityDelta& Delta : PlayerControllerSubViewDelta.EntityDeltas)
	{
		const Worker_EntityId& EntityId = Delta.EntityId;
		switch (Delta.Type)
		{
		case EntityDelta::ADD:
		{
			const EntityViewElement& Element = PlayerControllerSubView->GetView()[EntityId];
			for (const ComponentData& ComponentData : Element.Components)
			{
				if (ComponentData.GetComponentId() == SpatialConstants::PLAYER_CONTROLLER_SERVER_COMPONENT_ID)
				{
					AuthServerWorkerEntityId = PlayerControllerServer(ComponentData.GetUnderlying()).AuthServerWorkerEntityID;
				}
			}
			break;
		}
		case EntityDelta::UPDATE:
		{
			for (const ComponentChange& Change : Delta.ComponentUpdates)
			{
				if (Change.ComponentId == SpatialConstants::PLAYER_CONTROLLER_SERVER_COMPONENT_ID)
				{
					PlayerControllerServer PlayerControllerData = PlayerControllerServer();
					PlayerControllerData.ApplyComponentUpdate(Change.Update);
					AuthServerWorkerEntityId = PlayerControllerData.AuthServerWorkerEntityID;
				}
			}
			break;
		}
		default:
			break;
		}
	}

	const FSubViewDelta& WellKnownSubViewDelta = WellKnownSubView->GetViewDelta();
	for (const EntityDelta& Delta : WellKnownSubViewDelta.EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::REMOVE:
			if (Delta.EntityId == AuthServerWorkerEntityId)
			{
				UE_LOG(LogServerCrashHandler, Error, TEXT("Client's authoritative server worker entity %lld was deleted"), Delta.EntityId);
			}
		default:
			break;
		}
	}
}

} // namespace SpatialGDK
