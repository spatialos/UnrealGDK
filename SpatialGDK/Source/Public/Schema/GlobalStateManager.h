//#pragma once
//
//#include <improbable/c_worker.h>
//#include <improbable/c_schema.h>
//
//#include "Utils/SchemaUtils.h"
//
//const Worker_ComponentId GLOBAL_STATE_MANAGER_COMPONENT_ID = 100007;
//
//struct GlobalStateManager : Component
//{
//	static const Worker_ComponentId ComponentId = GLOBAL_STATE_MANAGER_COMPONENT_ID;
//
//	GlobalStateManager() = default;
//
//	GlobalStateManager(const Worker_ComponentData& Data)
//	{
//		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
//
//		SingletonNameToEntityId = Schema_GetStringToEntityMap(ComponentObject, 1);
//		StablyNamedPathToEntityId = Schema_GetStringToEntityMap(ComponentObject, 2);
//	}
//
//	Worker_ComponentData CreateGlobalStateManagerData()
//	{
//		Worker_ComponentData Data = {};
//		Data.component_id = GLOBAL_STATE_MANAGER_COMPONENT_ID;
//		Data.schema_type = Schema_CreateComponentData(GLOBAL_STATE_MANAGER_COMPONENT_ID);
//		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
//
//		Schema_AddStringToEntityMap(ComponentObject, 1, SingletonNameToEntityId);
//		Schema_AddStringToEntityMap(ComponentObject, 2, StablyNamedPathToEntityId);
//
//		return Data;
//	}
//
//	StringToEntityMap SingletonNameToEntityId;
//	StringToEntityMap StablyNamedPathToEntityId;
//};
