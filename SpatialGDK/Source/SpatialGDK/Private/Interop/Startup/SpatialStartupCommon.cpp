#include "Interop/Startup/SpatialStartupCommon.h"

#include "WorkerSDK/improbable/c_schema.h"

#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialStartupHandler);

namespace SpatialGDK
{
bool FDeploymentMapData::TryRead(const Schema_Object& Object, FDeploymentMapData& OutData)
{
	int FieldsFound = 0;

	Schema_Object* ComponentObject = &const_cast<Schema_Object&>(Object);

	if (Schema_GetBytesCount(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_MAP_URL_ID) != 0)
	{
		++FieldsFound;
		OutData.DeploymentMapURL = GetStringFromSchema(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_MAP_URL_ID);
	}

	if (Schema_GetBoolCount(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID) != 0)
	{
		++FieldsFound;
		OutData.bAcceptingPlayers = GetBoolFromSchema(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID);
	}
	if (Schema_GetInt32Count(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_SESSION_ID) != 0)
	{
		++FieldsFound;
		OutData.DeploymentSessionId = Schema_GetInt32(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_SESSION_ID);
	}
	if (Schema_GetUint32Count(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_SCHEMA_HASH) != 0)
	{
		++FieldsFound;
		OutData.SchemaHash = Schema_GetUint32(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_SCHEMA_HASH);
	}
	return FieldsFound == 4;
}
} // namespace SpatialGDK
