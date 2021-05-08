#include "LoadBalancing/LoadBalancingCalculator.h"
#include "Schema/StandardLibrary.h"
#include "Utils/SchemaUtils.h"

FGridBalancingCalculator::FGridBalancingCalculator(uint32 GridX, uint32 GridY, float Height, float Width)
	: Rows(GridY)
	, Cols(GridX)
	, WorldWidth(Width)
	, WorldHeight(Height)
{
	ComponentsToInspect.Add(SpatialConstants::POSITION_COMPONENT_ID);
}

void FGridBalancingCalculator::OnAdded_ReadComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data)
{
	Schema_Object* PositionObject = Schema_GetComponentDataFields(Data);

	Schema_Object* CoordsObject = Schema_GetObject(PositionObject, 1);
	SpatialGDK::Coordinates Coords;
	Coords.X = Schema_GetDouble(CoordsObject, 1);
	Coords.Y = Schema_GetDouble(CoordsObject, 2);
	Coords.Z = Schema_GetDouble(CoordsObject, 3);

	FVector Position = SpatialGDK::Coordinates::ToFVector(Coords);

	Modified.Add(EntityId);
	EntityData NewEntityData;
	NewEntityData.Position = Position;
	Positions.Add(EntityId, NewEntityData);
}

void FGridBalancingCalculator::OnRemoved(Worker_EntityId EntityId)
{
	Modified.Remove(EntityId);
	Positions.Remove(EntityId);
}

void FGridBalancingCalculator::OnUpdate(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update)
{
	if (ComponentId == SpatialConstants::POSITION_COMPONENT_ID)
	{
		Schema_Object* PositionObject = Schema_GetComponentUpdateFields(Update);
		Schema_Object* CoordsObject = Schema_GetObject(PositionObject, 1);
		SpatialGDK::Coordinates Coords;
		Coords.X = Schema_GetDouble(CoordsObject, 1);
		Coords.Y = Schema_GetDouble(CoordsObject, 2);
		Coords.Z = Schema_GetDouble(CoordsObject, 3);

		EntityData& Data = Positions.FindChecked(EntityId);
		Data.Position = SpatialGDK::Coordinates::ToFVector(Coords);

		Modified.Add(EntityId);
	}
}

void FGridBalancingCalculator::CollectPartitionsToAdd(const FPartitionDeclaration* Parent,
													  TArray<TSharedPtr<FPartitionDeclaration>>& OutPartitions)
{
	if (Partitions.Num() == 0)
	{
		const float WorldWidthMin = -(WorldWidth / 2.f);
		const float WorldHeightMin = -(WorldHeight / 2.f);

		const float ColumnWidth = WorldWidth / Cols;
		const float RowHeight = WorldHeight / Rows;

		// We would like the inspector's representation of the load balancing strategy to match our intuition.
		// +x is forward, so rows are perpendicular to the x-axis and columns are perpendicular to the y-axis.
		float XMin = WorldHeightMin;
		float YMin = WorldWidthMin;
		float XMax, YMax;

		for (uint32 Col = 0; Col < Cols; ++Col)
		{
			YMax = YMin + ColumnWidth;

			for (uint32 Row = 0; Row < Rows; ++Row)
			{
				XMax = XMin + RowHeight;

				FVector2D Min(XMin, YMin);
				FVector2D Max(XMax, YMax);
				FBox2D Cell(Min, Max);
				Cells.Add(Cell);

				TSharedPtr<FPartitionDeclaration> NewPartition = MakeShared<FPartitionDeclaration>();
				NewPartition->bActive = true;
				NewPartition->Calculator = this;
				NewPartition->CalculatorIndex = Partitions.Num();
				NewPartition->ParentPartition = Parent;
				NewPartition->Name = FString::Printf(TEXT("GridCell (%i, %i)"), Col, Row);

				Partitions.Add(NewPartition);

				OutPartitions.Add(NewPartition);

				XMin = XMax;
			}

			XMin = WorldHeightMin;
			YMin = YMax;
		}
	}
}

void FGridBalancingCalculator::CollectEntitiesToMigrate(MigrationContext& Ctx)
{
	TSet<Worker_EntityId> NotChecked;
	for (Worker_EntityId EntityId : Modified)
	{
		if (Ctx.MigratingEntities.Contains(EntityId))
		{
			NotChecked.Add(EntityId);
			continue;
		}

		EntityData& Data = Positions.FindChecked(EntityId);

		const FVector2D Actor2DLocation(Data.Position);

		if (Data.Assignment >= 0 && Data.Assignment < Cells.Num())
		{
			if (Cells[Data.Assignment].IsInside(Actor2DLocation))
			{
				continue;
			}
		}
		int32 NewAssignment = -1;
		for (int i = 0; i < Cells.Num(); i++)
		{
			if (Cells[i].IsInside(Actor2DLocation))
			{
				NewAssignment = i;
			}
		}

		if (NewAssignment >= 0 && NewAssignment < Cells.Num() && ensure(NewAssignment != Data.Assignment))
		{
			Data.Assignment = NewAssignment;
			Ctx.EntitiesToMigrate.Add(EntityId, Partitions[NewAssignment]);
		}
	}
	Modified = MoveTemp(NotChecked);
}

FLayerLoadBalancingCalculator::FLayerLoadBalancingCalculator(TArray<FName> InLayerNames,
															 TArray<TUniquePtr<FLoadBalancingCalculator>>&& InLayers)
	: LayerNames(InLayerNames)
	, Layers(MoveTemp(InLayers))
{
	ComponentsToInspect.Add(SpatialConstants::ACTOR_GROUP_MEMBER_COMPONENT_ID);
}

void FLayerLoadBalancingCalculator::OnAdded(Worker_EntityId EntityId, SpatialGDK::EntityViewElement const& Element)
{
	FLoadBalancingCalculator::OnAdded(EntityId, Element);
	uint32* Group = GroupMembership.Find(EntityId);
	if (Group)
	{
		Layers[*Group]->OnAdded(EntityId, Element);
	}
}

void FLayerLoadBalancingCalculator::OnAdded_ReadComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId,
														  Schema_ComponentData* Data)
{
	Schema_Object* GroupObject = Schema_GetComponentDataFields(Data);
	int32 GroupId = Schema_GetUint32(GroupObject, SpatialConstants::ACTOR_GROUP_MEMBER_COMPONENT_ACTOR_GROUP_ID);
	if (GroupId >= 0 && GroupId < Layers.Num())
	{
		GroupMembership.Add(EntityId, GroupId);
	}
}

void FLayerLoadBalancingCalculator::OnRemoved(Worker_EntityId EntityId)
{
	uint32 Group;
	GroupMembership.RemoveAndCopyValue(EntityId, Group);

	Layers[Group]->OnRemoved(EntityId);
}

void FLayerLoadBalancingCalculator::OnUpdate(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update)
{
	// Snore.... try to organize a data oriented update dispatch instead.
	// Even though different calculator instances manage different entities they could share data storage.
	uint32* Group = GroupMembership.Find(EntityId);
	if (Group)
	{
		Layers[*Group]->OnUpdate(EntityId, ComponentId, Update);
	}
}

void FLayerLoadBalancingCalculator::CollectPartitionsToAdd(const FPartitionDeclaration* Parent,
														   TArray<TSharedPtr<FPartitionDeclaration>>& OutPartitions)
{
	for (int32 i = 0; i < Layers.Num(); ++i)
	{
		FLoadBalancingCalculator* Calculator = Layers[i].Get();
		TSharedPtr<FPartitionDeclaration> NewPartition = MakeShared<FPartitionDeclaration>();
		NewPartition->bActive = true;
		NewPartition->Calculator = this;
		NewPartition->CalculatorIndex = VirtualPartitions.Num();
		NewPartition->ParentPartition = Parent;
		NewPartition->Name = FString::Printf(TEXT("Layer %s"), *LayerNames[i].ToString());
		VirtualPartitions.Add(NewPartition);

		Layers[i]->CollectPartitionsToAdd(NewPartition.Get(), OutPartitions);
	}
}

void FLayerLoadBalancingCalculator::CollectEntitiesToMigrate(MigrationContext& Ctx)
{
	for (TUniquePtr<FLoadBalancingCalculator>& Calculator : Layers)
	{
		Calculator->CollectEntitiesToMigrate(Ctx);
	}
}

void FLayerLoadBalancingCalculator::CollectComponentsToInspect(TSet<Worker_ComponentId>& OutSet)
{
	FLoadBalancingCalculator::CollectComponentsToInspect(OutSet);
	for (TUniquePtr<FLoadBalancingCalculator>& Calculator : Layers)
	{
		Calculator->CollectComponentsToInspect(OutSet);
	}
}
