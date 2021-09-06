#pragma once

#include "Schema/StandardLibrary.h"
#include "SpatialView/ComponentData.h"
#include "SpatialView/ComponentUpdate.h"

namespace SpatialGDK
{
struct LegacyLB_GridCell
{
	static constexpr Worker_ComponentId ComponentId = 1901;

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
	static constexpr Worker_ComponentId ComponentId = 1902;

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
	static constexpr Worker_ComponentId ComponentId = 1903;

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

struct LegacyLB_CustomWorkerAssignments
{
	static constexpr Worker_ComponentId ComponentId = 1904;

	LegacyLB_CustomWorkerAssignments() = default;

	explicit LegacyLB_CustomWorkerAssignments(const ComponentData& Data)
		: LegacyLB_CustomWorkerAssignments(Data.GetUnderlying())
	{
	}

	explicit LegacyLB_CustomWorkerAssignments(Schema_ComponentData* Data)
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
		LabelToVirtualWorker.Empty();
		AdditionalInterest.Empty();
		uint32 NumEntries = Schema_GetUint32Count(Object, 1);
		uint32 NumStrings = Schema_GetBytesCount(Object, 2);
		for (uint32 i = 0; i < NumEntries; ++i)
		{
			uint32_t WorkerId = Schema_IndexUint32(Object, 1, i);
			FString Label = IndexStringFromSchema(Object, 2, i);
			LabelToVirtualWorker.Add(FName(*Label), WorkerId);
		}
		uint32 NumEntriesInterest = Schema_GetBytesCount(Object, 3);
		for (uint32 i = 0; i < NumEntriesInterest; ++i)
		{
			FString Label = IndexStringFromSchema(Object, 3, i);
			AdditionalInterest.Add(FName(*Label));
		}
	}

	void WriteToObject(Schema_Object* Object) const
	{
		for (const auto& Entry : LabelToVirtualWorker)
		{
			Schema_AddUint32(Object, 1, Entry.Value);
			AddStringToSchema(Object, 2, Entry.Key.ToString());
		}
		for (const auto& Label : AdditionalInterest)
		{
			AddStringToSchema(Object, 3, Label.ToString());
		}
	}

	TMap<FName, uint32> LabelToVirtualWorker;
	TSet<FName> AdditionalInterest;
};
} // namespace SpatialGDK
