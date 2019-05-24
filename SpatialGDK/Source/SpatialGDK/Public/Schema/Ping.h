// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
	struct ServerPing : Component
	{
		static const Worker_ComponentId ComponentId = SpatialConstants::SERVER_PING_COMPONENT_ID;

		ServerPing() = default;
		ServerPing(const Worker_ComponentData& Data)
		{
		}

		inline Worker_ComponentData CreateServerPingData()
		{
			Worker_ComponentData Data = {};
			Data.component_id = ComponentId;
			Data.schema_type = Schema_CreateComponentData(ComponentId);

			return Data;
		}
	};

	struct ClientPong : Component
	{
		static const Worker_ComponentId ComponentId = SpatialConstants::CLIENT_PONG_COMPONENT_ID;

		ClientPong() = default;
		ClientPong(const Worker_ComponentData& Data)
		{
		}

		inline Worker_ComponentData CreateClientPongData()
		{
			Worker_ComponentData Data = {};
			Data.component_id = ComponentId;
			Data.schema_type = Schema_CreateComponentData(ComponentId);

			return Data;
		}
	};
} // namespace SpatialGDK
