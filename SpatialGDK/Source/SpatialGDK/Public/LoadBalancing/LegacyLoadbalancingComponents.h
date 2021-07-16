#pragma once

#include "Schema/StandardLibrary.h"
#include "SpatialView/ComponentData.h"
#include "SpatialView/ComponentUpdate.h"

namespace SpatialGDK
{
struct LegacyLB_GridCell
{
	static const Worker_ComponentId ComponentId = 190502;

	LegacyLB_GridCell() {}

	LegacyLB_GridCell(const ComponentData& Data)
		: LegacyLB_GridCell(Data.GetUnderlying())
	{
	}

	LegacyLB_GridCell(Schema_ComponentData* Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data);
		ReadFromObject(ComponentObject);
	}

	ComponentData CreateComponentData() const
	{
		ComponentData Data(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.GetUnderlying());

		WriteToObject(ComponentObject);

		return Data;
	}

	ComponentUpdate CreateComponentUpdate() const
	{
		ComponentUpdate Update(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.GetUnderlying());

		WriteToObject(ComponentObject);

		return Update;
	}

	void ApplyComponentUpdate(const ComponentUpdate& Update) { ApplyComponentUpdate(Update.GetUnderlying()); }

	void ApplyComponentUpdate(Schema_ComponentUpdate* Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update);
		ReadFromObject(ComponentObject);
	}

	void ReadFromObject(Schema_Object* Object)
	{
		Center = Coordinates::ToFVector(GetCoordinateFromSchema(Object, 1));
		Edge_length = Coordinates::ToFVector(GetCoordinateFromSchema(Object, 2));
	}

	void WriteToObject(Schema_Object* Object) const
	{
		AddCoordinateToSchema(Object, 1, Coordinates::FromFVector(Center));
		AddCoordinateToSchema(Object, 2, Coordinates::FromFVector(Edge_length));
	}

	FVector Center;
	FVector Edge_length;
};

struct LegacyLB_Layer
{
	static const Worker_ComponentId ComponentId = 190501;

	LegacyLB_Layer() {}

	LegacyLB_Layer(const ComponentData& Data)
		: LegacyLB_Layer(Data.GetUnderlying())
	{
	}

	LegacyLB_Layer(Schema_ComponentData* Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data);
		ReadFromObject(ComponentObject);
	}

	ComponentData CreateComponentData() const
	{
		ComponentData Data(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.GetUnderlying());

		WriteToObject(ComponentObject);

		return Data;
	}

	ComponentUpdate CreateComponentUpdate() const
	{
		ComponentUpdate Update(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.GetUnderlying());

		WriteToObject(ComponentObject);

		return Update;
	}

	void ApplyComponentUpdate(const ComponentUpdate& Update) { ApplyComponentUpdate(Update.GetUnderlying()); }

	void ApplyComponentUpdate(Schema_ComponentUpdate* Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update);
		ReadFromObject(ComponentObject);
	}

	void ReadFromObject(Schema_Object* Object)
	{
		if (Schema_GetUint32Count(Object, 1) > 0)
		{
			Layer = Schema_GetUint32(Object, 1);
		}
	}

	void WriteToObject(Schema_Object* Object) const { Schema_AddUint32(Object, 1, Layer); }

	uint32 Layer;
};

struct LegacyLB_VirtualWorkerAssignment
{
	static const Worker_ComponentId ComponentId = 190500;

	LegacyLB_VirtualWorkerAssignment() {}

	LegacyLB_VirtualWorkerAssignment(const ComponentData& Data)
		: LegacyLB_VirtualWorkerAssignment(Data.GetUnderlying())
	{
	}

	LegacyLB_VirtualWorkerAssignment(Schema_ComponentData* Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data);
		ReadFromObject(ComponentObject);
	}

	ComponentData CreateComponentData() const
	{
		ComponentData Data(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.GetUnderlying());

		WriteToObject(ComponentObject);

		return Data;
	}

	ComponentUpdate CreateComponentUpdate() const
	{
		ComponentUpdate Update(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.GetUnderlying());

		WriteToObject(ComponentObject);

		return Update;
	}

	void ApplyComponentUpdate(const ComponentUpdate& Update) { ApplyComponentUpdate(Update.GetUnderlying()); }

	void ApplyComponentUpdate(Schema_ComponentUpdate* Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update);
		ReadFromObject(ComponentObject);
	}

	void ReadFromObject(Schema_Object* Object)
	{
		if (Schema_GetUint32Count(Object, 1) > 0)
		{
			Virtual_worker_id = Schema_GetUint32(Object, 1);
		}
	}

	void WriteToObject(Schema_Object* Object) const { Schema_AddUint32(Object, 1, Virtual_worker_id); }

	uint32 Virtual_worker_id;
};
} // namespace SpatialGDK
