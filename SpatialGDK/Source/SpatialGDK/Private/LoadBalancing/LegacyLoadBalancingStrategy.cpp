// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/LegacyLoadBalancingStrategy.h"

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/SpatialOSWorkerInterface.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "LoadBalancing/LBDataStorage.h"
#include "LoadBalancing/PartitionManager.h"

namespace SpatialGDK
{
FLegacyLoadBalancing::FLegacyLoadBalancing(UAbstractLBStrategy& LegacyLBStrat, SpatialVirtualWorkerTranslator& InTranslator)
	: Translator(InTranslator)
{
	ExpectedWorkers = LegacyLBStrat.GetMinimumRequiredWorkers();
	VirtualWorkerIdToHandle.SetNum(ExpectedWorkers);

	bDirectAssignment = !LegacyLBStrat.IsStrategyWorkerAware();
	if (bDirectAssignment)
	{
		AssignmentStorage = MakeUnique<FDirectAssignmentStorage>();
	}
	else
	{
		PositionStorage = MakeUnique<SpatialGDK::FSpatialPositionStorage>();
		GroupStorage = MakeUnique<SpatialGDK::FActorGroupStorage>();
		LegacyLBStrat.GetLegacyLBInformation(LBContext);
	}
}

FLegacyLoadBalancing::~FLegacyLoadBalancing() {}

void FLegacyLoadBalancing::Advance(ISpatialOSWorker& Connection)
{
	const TArray<Worker_Op>& Messages = Connection.GetWorkerMessages();

	for (const auto& Message : Messages)
	{
		switch (Message.op_type)
		{
		case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
		{
			const Worker_EntityQueryResponseOp& Op = Message.op.entity_query_response;
			if (WorkerTranslationRequest.IsSet() && Op.request_id == WorkerTranslationRequest.GetValue())
			{
				if (Op.status_code == WORKER_STATUS_CODE_SUCCESS)
				{
					for (uint32_t i = 0; i < Op.results[0].component_count; i++)
					{
						Worker_ComponentData Data = Op.results[0].components[i];
						if (Data.component_id == SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID)
						{
							Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
							Translator.ApplyVirtualWorkerManagerData(ComponentObject);
							bTranslatorIsReady = true;
							for (uint32 VirtualWorker = 0; VirtualWorker < ExpectedWorkers; ++VirtualWorker)
							{
								if (Translator.GetServerWorkerEntityForVirtualWorker(VirtualWorker + 1)
									== SpatialConstants::INVALID_ENTITY_ID)
								{
									bTranslatorIsReady = false;
								}
							}
						}
					}
				}

				WorkerTranslationRequest.Reset();
			}
		}
		}
	}
}

void FLegacyLoadBalancing::QueryTranslation(ISpatialOSWorker& Connection)
{
	if (WorkerTranslationRequest.IsSet())
	{
		return;
	}

	// Build a constraint for the Virtual Worker Translation.
	Worker_ComponentConstraint TranslationComponentConstraint;
	TranslationComponentConstraint.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;

	Worker_Constraint TranslationConstraint;
	TranslationConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_COMPONENT;
	TranslationConstraint.constraint.component_constraint = TranslationComponentConstraint;

	Worker_EntityQuery TranslationQuery{};
	TranslationQuery.constraint = TranslationConstraint;

	WorkerTranslationRequest = Connection.SendEntityQueryRequest(EntityQuery(TranslationQuery), RETRY_UNTIL_COMPLETE);
}

void FLegacyLoadBalancing::Flush(ISpatialOSWorker& Connection)
{
	if (ConnectedWorkers.Num() == ExpectedWorkers && !bTranslatorIsReady)
	{
		QueryTranslation(Connection);
	}
}

void FLegacyLoadBalancing::Init(TArray<FLBDataStorage*>& OutLoadBalancingData)
{
	if (PositionStorage)
	{
		OutLoadBalancingData.Add(PositionStorage.Get());
	}
	if (GroupStorage)
	{
		OutLoadBalancingData.Add(GroupStorage.Get());
	}
	if (AssignmentStorage)
	{
		OutLoadBalancingData.Add(AssignmentStorage.Get());
	}
}

void FLegacyLoadBalancing::OnWorkersConnected(TArrayView<FLBWorkerHandle> InConnectedWorkers)
{
	for (auto& Worker : InConnectedWorkers)
	{
		ConnectedWorkers.Add(Worker);
	}
}

void FLegacyLoadBalancing::OnWorkersDisconnected(TArrayView<FLBWorkerHandle> DisconnectedWorkers)
{
	// Not handled
}

void FLegacyLoadBalancing::TickPartitions(FPartitionManager& PartitionMgr)
{
	if (bCreatedPartitions)
	{
		return;
	}
	if (ConnectedWorkers.Num() == ExpectedWorkers && bTranslatorIsReady)
	{
		TMap<Worker_EntityId_Key, FLBWorkerHandle> WorkersMap;
		for (auto Worker : ConnectedWorkers)
		{
			WorkersMap.Add(PartitionMgr.GetServerWorkerEntityIdForWorker(Worker), Worker);
		}

		for (VirtualWorkerId i = 1; i <= ExpectedWorkers; ++i)
		{
			Worker_EntityId ServerWorkerEntityId = Translator.GetServerWorkerEntityForVirtualWorker(i);
			VirtualWorkerIdToHandle[i - 1] = WorkersMap.FindChecked(ServerWorkerEntityId);
		}

		if (!bDirectAssignment)
		{
			Partitions.SetNum(ExpectedWorkers);
			for (const auto& Layer : LBContext.Layers)
			{
				for (const auto& Cell : Layer.Cells)
				{
					FVector2D CellCenter = Cell.Region.GetCenter();
					const FVector Center3D{ CellCenter.X, CellCenter.Y, 0.0f };

					const FVector2D EdgeLengths2D = Cell.Region.GetSize();
					check(EdgeLengths2D.X > 0.0f && EdgeLengths2D.Y > 0.0f);
					const FVector EdgeLengths3D{ EdgeLengths2D.X + Cell.Border, EdgeLengths2D.Y + Cell.Border, FLT_MAX };

					SpatialGDK::QueryConstraint Constraint;
					Constraint.BoxConstraint = SpatialGDK::BoxConstraint{ SpatialGDK::Coordinates::FromFVector(Center3D),
																		  SpatialGDK::EdgeLength::FromFVector(EdgeLengths3D) };

					FString PartitionName =
						FString::Printf(TEXT("Layer : %s, Cell (%f,%f)"), *Layer.Name.ToString(), CellCenter.X, CellCenter.Y);
					Partitions[Cell.WorkerId - 1] = PartitionMgr.CreatePartition(PartitionName, nullptr, Constraint);
				}
			}
		}
		else
		{
			for (VirtualWorkerId i = 1; i <= ExpectedWorkers; ++i)
			{
				Partitions.Add(
					PartitionMgr.CreatePartition(FString::Printf(TEXT("VirtualWorker Partition %i"), i), nullptr, QueryConstraint()));
			}
		}

		for (uint32 i = 0; i < ExpectedWorkers; ++i)
		{
			PartitionMgr.AssignPartitionTo(Partitions[i], VirtualWorkerIdToHandle[i]);
		}
		bCreatedPartitions = true;
	}
}

void FLegacyLoadBalancing::CollectEntitiesToMigrate(FMigrationContext& Ctx)
{
	if (bCreatedPartitions)
	{
		if (!bDirectAssignment)
		{
			TSet<Worker_EntityId_Key> NotChecked;
			ToRefresh = ToRefresh.Union(Ctx.ModifiedEntities);
			for (Worker_EntityId EntityId : ToRefresh)
			{
				if (Ctx.MigratingEntities.Contains(EntityId))
				{
					NotChecked.Add(EntityId);
					continue;
				}

				int32 Group = GroupStorage->GetGroups().FindChecked(EntityId);
				FLegacyLBContext::Layer& Layer = LBContext.Layers[Group];

				const FVector& Position = PositionStorage->GetPositions().FindChecked(EntityId);
				const FVector2D Actor2DLocation(Position);

				int32& CurAssignment = Assignment.FindOrAdd(EntityId, -1);

				if (CurAssignment >= 0 && CurAssignment < Layer.Cells.Num())
				{
					if (Layer.Cells[CurAssignment].Region.IsInside(Actor2DLocation))
					{
						continue;
					}
				}

				int32 NewAssignment = -1;
				for (const auto& CandidateCell : Layer.Cells)
				{
					if (CandidateCell.Region.IsInside(Actor2DLocation))
					{
						NewAssignment = CandidateCell.WorkerId - 1;
					}
				}

				if (NewAssignment >= 0 && NewAssignment < Partitions.Num() && ensureAlways(NewAssignment != CurAssignment))
				{
					CurAssignment = NewAssignment;
					Ctx.EntitiesToMigrate.Add(EntityId, Partitions[CurAssignment]);
				}
			}
			ToRefresh = MoveTemp(NotChecked);
		}
		else
		{
			const TMap<Worker_EntityId_Key, AuthorityIntent>& AssignmentMap = AssignmentStorage->GetObjects();
			for (Worker_EntityId ToMigrate : Ctx.ModifiedEntities)
			{
				if (!ensureAlways(!Ctx.MigratingEntities.Contains(ToMigrate)))
				{
					continue;
				}
				const AuthorityIntent& Intent = AssignmentMap.FindChecked(ToMigrate);
				if (!ensureAlways(Intent.VirtualWorkerId > 0 && Intent.VirtualWorkerId <= (uint32)Partitions.Num()))
				{
					continue;
				}
				Ctx.EntitiesToMigrate.Add(ToMigrate, Partitions[Intent.VirtualWorkerId - 1]);
			}
		}
	}
}
} // namespace SpatialGDK
