// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/LegacyLoadBalancingStrategy.h"

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/SpatialOSWorkerInterface.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "LoadBalancing/ActorSetSystem.h"
#include "LoadBalancing/InterestManager.h"
#include "LoadBalancing/LBDataStorage.h"
#include "LoadBalancing/LegacyLoadBalancingCommon.h"
#include "LoadBalancing/LegacyLoadbalancingComponents.h"
#include "LoadBalancing/PartitionManager.h"
#include "Schema/DebugComponent.h"

DEFINE_LOG_CATEGORY(LogSpatialLegacyLoadBalancing)

namespace SpatialGDK
{
class FActorGroupStorage : public TLBDataStorage<ActorGroupMember>
{
};

class FDirectAssignmentStorage : public TLBDataStorage<AuthorityIntent>
{
};

class FDebugComponentStorage : public TLBDataStorage<DebugComponent>
{
};

class FCustomWorkerAssignmentStorage : public TLBDataStorage<LegacyLB_CustomWorkerAssignments>
{
};

class DbgPositionStorage : public FSpatialPositionStorage
{
public:
	DbgPositionStorage(FBox2D Region, uint32 Num)
	{
		FVector2D Offset = Region.Min;
		FVector2D Size = Region.GetSize();
		for (uint32 i = 0; i < Num; ++i)
		{
			Modified.Add(i + 1);

			FVector Pos(FMath::RandRange(0.0f, Size.X) + Offset.X, FMath::RandRange(0.0f, Size.Y) + Offset.Y, 0);

			Positions.Add(i + 1, Pos);
		}
	}
};

static const FBox2D TestBox = FBox2D(FVector2D(-100000, -100000), FVector2D(100000, 100000));

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
		PositionStorage = MakeUnique<FSpatialPositionStorage>();
		GroupStorage = MakeUnique<FActorGroupStorage>();
#if !UE_BUILD_SHIPPING
		DebugCompStorage = MakeUnique<FDebugComponentStorage>();

		ServerWorkerCustomAssignment = MakeUnique<FCustomWorkerAssignmentStorage>();
#endif //! UE_BUILD_SHIPPING

		LegacyLBStrat.GetLegacyLBInformation(LBContext);
	}

	AlwaysRelevantStorage = MakeUnique<FAlwaysRelevantStorage>();
	ServerAlwaysRelevantStorage = MakeUnique<FServerAlwaysRelevantStorage>();
}

FLegacyLoadBalancing::~FLegacyLoadBalancing() {}

void FLegacyLoadBalancing::Advance(ISpatialOSWorker& Connection, const TSet<Worker_EntityId_Key>& DeletedEntities)
{
	CommandsHandler.ProcessOps(Connection.GetWorkerMessages());
	if (InterestManager)
	{
		InterestManager->Advance(DeletedEntities);
		// InterestManager->Advance(TSet<Worker_EntityId_Key>());
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
	CommandsHandler.AddRequest(*WorkerTranslationRequest, [this](const Worker_EntityQueryResponseOp& Op) {
		if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
		{
			return;
		}
		WorkerTranslationRequest.Reset();

		for (uint32_t i = 0; i < Op.results[0].component_count; i++)
		{
			Worker_ComponentData Data = Op.results[0].components[i];
			if (Data.component_id == SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID)
			{
				Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
				Translator.ApplyMappingFromSchema(ComponentObject);
				bTranslatorIsReady = true;
				for (uint32 VirtualWorker = 0; VirtualWorker < ExpectedWorkers; ++VirtualWorker)
				{
					if (Translator.GetServerWorkerEntityForVirtualWorker(VirtualWorker + 1) == SpatialConstants::INVALID_ENTITY_ID)
					{
						bTranslatorIsReady = false;
					}
				}
			}
		}
	});
}

void FLegacyLoadBalancing::Flush(ISpatialOSWorker& Connection)
{
	if (ConnectedWorkers.Num() == ExpectedWorkers && !bTranslatorIsReady)
	{
		QueryTranslation(Connection);
	}

	if (InterestManager)
	{
		TArray<Worker_EntityId> Workers;
		TArray<FBox2D> Regions;

		// const uint32 div = 8;
		// FVector2D Increment = (TestBox.GetSize() / div) * 1.05;
		// FVector2D Start = TestBox.Min;
		// for (uint32 x = 0; x < div; ++x)
		//{
		//	for (uint32 y = 0; y < div; ++y)
		//	{
		//		FVector2D Offset = Start;
		//		Offset.X += x * Increment.X;
		//		Offset.Y += x * Increment.Y;
		//		Regions.Add(FBox2D(Offset, Offset + Increment));
		//		Workers.Add(Workers.Num());
		//	}
		//}

		for (const auto& Layer : LBContext.Layers)
		{
			for (const auto& Cell : Layer.Cells)
			{
				Worker_EntityId SystemEntity =
					SharedData->PartitionManager.GetSystemWorkerEntityIdForWorker(VirtualWorkerIdToHandle[Cell.WorkerId - 1]);

				Workers.Add(SystemEntity);
				Regions.Add(Cell.Region);
				Regions.Last().Min.X -= Cell.Border;
				Regions.Last().Min.Y -= Cell.Border;
				Regions.Last().Max.X += Cell.Border;
				Regions.Last().Max.Y += Cell.Border;
			}
		}

		InterestManager->ComputeInterest(Connection, Workers, Regions);
	}
}

void FLegacyLoadBalancing::Init(FLoadBalancingSharedData InSharedData, TArray<FLBDataStorage*>& OutLoadBalancingData,
								TArray<FLBDataStorage*>& OutServerWorkerData)
{
	SharedData.Emplace(InSharedData);

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
#if !UE_BUILD_SHIPPING
	if (DebugCompStorage)
	{
		OutLoadBalancingData.Add(DebugCompStorage.Get());
	}

	if (ServerWorkerCustomAssignment)
	{
		OutServerWorkerData.Add(ServerWorkerCustomAssignment.Get());
	}
#endif //! UE_BUILD_SHIPPING

	OutLoadBalancingData.Add(AlwaysRelevantStorage.Get());
	OutLoadBalancingData.Add(ServerAlwaysRelevantStorage.Get());

	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();
	if (Settings->bUserSpaceServerInterest)
	{
		InterestManager =
			MakeUnique<FInterestManager>(SharedData->InterestF, *PositionStorage, *AlwaysRelevantStorage, *ServerAlwaysRelevantStorage);
	}

	// InterestManager = MakeUnique<FInterestManager>(SharedData->InterestF,
	//*new DbgPositionStorage(TestBox, 210000), *new FAlwaysRelevantStorage, *new FServerAlwaysRelevantStorage);
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
void FLegacyLoadBalancing::TickPartitions()
{
	FPartitionManager& PartitionMgr = SharedData->PartitionManager;

	if (bCreatedPartitions)
	{
#if !UE_BUILD_SHIPPING
		if (ServerWorkerCustomAssignment->GetModifiedEntities().Num() > 0 || DebugCompStorage->GetModifiedEntities().Num() > 0)
		{
			for (const auto& Layer : LBContext.Layers)
			{
				for (const auto& Cell : Layer.Cells)
				{
					const uint32 WorkerIdx = Cell.WorkerId - 1;
					FPartitionHandle WorkerPartition = Partitions[WorkerIdx];
					FLBWorkerHandle WorkerHandle = VirtualWorkerIdToHandle[WorkerIdx];
					Worker_EntityId ServerWorkerEntity = PartitionMgr.GetServerWorkerEntityIdForWorker(WorkerHandle);

					const LegacyLB_CustomWorkerAssignments* DebugWorkerInfo =
						ServerWorkerCustomAssignment->GetObjects().Find(ServerWorkerEntity);
					TSet<Worker_EntityId_Key> AdditionalEntities;
					if (DebugWorkerInfo)
					{
						for (const auto& DebugEntry : DebugCompStorage->GetObjects())
						{
							for (const FName& Label : DebugEntry.Value.ActorTags)
							{
								if (DebugWorkerInfo->AdditionalInterest.Contains(Label))
								{
									AdditionalEntities.Add(DebugEntry.Key);
									break;
								}
							}
						}
					}

					const FVector2D CellCenter = Cell.Region.GetCenter();
					const FVector Center3D{ CellCenter.X, CellCenter.Y, 0.0f };

					const FVector2D EdgeLengths2D = Cell.Region.GetSize();
					check(EdgeLengths2D.X > 0.0f && EdgeLengths2D.Y > 0.0f);
					const FVector EdgeLengths3D{ EdgeLengths2D.X + Cell.Border, EdgeLengths2D.Y + Cell.Border, FLT_MAX };

					SpatialGDK::QueryConstraint Constraint;
					Constraint.BoxConstraint = SpatialGDK::BoxConstraint{ SpatialGDK::Coordinates::FromFVector(Center3D),
																		  SpatialGDK::EdgeLength::FromFVector(EdgeLengths3D) };

					if (AdditionalEntities.Num() > 0)
					{
						SpatialGDK::QueryConstraint EntitiesConstraint;
						EntitiesConstraint.OrConstraint.Reserve(AdditionalEntities.Num());
						for (Worker_EntityId Entity : AdditionalEntities)
						{
							SpatialGDK::QueryConstraint EntityQuery;
							EntityQuery.EntityIdConstraint = Entity;
							EntitiesConstraint.OrConstraint.Add(EntityQuery);
						}
						SpatialGDK::QueryConstraint CompleteConstraint;
						CompleteConstraint.OrConstraint.Add(Constraint);
						CompleteConstraint.OrConstraint.Add(EntitiesConstraint);
						PartitionMgr.SetPartitionInterest(WorkerPartition, CompleteConstraint);
					}
					else
					{
						PartitionMgr.SetPartitionInterest(WorkerPartition, Constraint);
					}
				}
			}
		}
#endif //! UE_BUILD_SHIPPING
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

					TArray<ComponentData> Components;

					LegacyLB_GridCell GridCellComp;
					GridCellComp.Center = Center3D;
					GridCellComp.Edge_length = FVector(EdgeLengths2D.X, EdgeLengths2D.Y, FLT_MAX);

					Components.Add(GridCellComp.CreateComponentData());

					LegacyLB_Layer LayerComp;
					LayerComp.Layer = &Layer - LBContext.Layers.GetData();

					Components.Add(LayerComp.CreateComponentData());

					LegacyLB_VirtualWorkerAssignment VirtualWorkerComp;
					VirtualWorkerComp.Virtual_worker_id = Cell.WorkerId;

					Components.Add(VirtualWorkerComp.CreateComponentData());

					Partitions[Cell.WorkerId - 1] = PartitionMgr.CreatePartition(PartitionName, nullptr, Constraint, MoveTemp(Components));
				}
			}
		}
		else
		{
			for (VirtualWorkerId i = 1; i <= ExpectedWorkers; ++i)
			{
				TArray<ComponentData> Components;

				LegacyLB_VirtualWorkerAssignment VirtualWorkerComp;
				VirtualWorkerComp.Virtual_worker_id = i;

				Components.Add(VirtualWorkerComp.CreateComponentData());

				Partitions.Add(PartitionMgr.CreatePartition(FString::Printf(TEXT("VirtualWorker Partition %i"), i), nullptr,
															QueryConstraint(), MoveTemp(Components)));
			}
		}

		for (uint32 i = 0; i < ExpectedWorkers; ++i)
		{
			PartitionMgr.AssignPartitionTo(Partitions[i], VirtualWorkerIdToHandle[i]);
		}

		for (auto Worker : ConnectedWorkers)
		{
			Worker_EntityId ServerWorkerEntity = PartitionMgr.GetServerWorkerEntityIdForWorker(Worker);
			if (WorkerForCustomAssignment == 0 || ServerWorkerEntity < WorkerForCustomAssignment)
			{
				WorkerForCustomAssignment = ServerWorkerEntity;
			}
		}
		bCreatedPartitions = true;
	}
}

TOptional<uint32> FLegacyLoadBalancing::EvaluateDebugComponent(Worker_EntityId EntityId)
{
	auto AssignmentData = ServerWorkerCustomAssignment->GetObjects().Find(WorkerForCustomAssignment);
	if (!AssignmentData)
	{
		return {};
	}

	if (const DebugComponent* DbgComp = DebugCompStorage->GetObjects().Find(EntityId))
	{
		if (DbgComp->DelegatedWorkerId.IsSet())
		{
			if (!ensure(int32(DbgComp->DelegatedWorkerId.GetValue() - 1) < Partitions.Num()))
			{
				return {};
			}
			return DbgComp->DelegatedWorkerId.GetValue();
		}

		FPartitionHandle CandidatePartition;
		uint32 CandidateWorker;
		for (auto Tag : DbgComp->ActorTags)
		{
			if (const uint32* WorkerId = AssignmentData->LabelToVirtualWorker.Find(Tag))
			{
				if (!ensure(int32(*WorkerId - 1) < Partitions.Num()))
				{
					continue;
				}
				FPartitionHandle CurPartition = Partitions[*WorkerId - 1];
				ensure(!CandidatePartition.IsValid() || CurPartition == CandidatePartition);
				CandidatePartition = CurPartition;
				CandidateWorker = *WorkerId;
			}
		}
		if (CandidatePartition.IsValid())
		{
			return CandidateWorker;
		}
	}
	return {};
}

TOptional<TPair<Worker_EntityId, uint32>> FLegacyLoadBalancing::EvaluateDebugComponentWithSet(Worker_EntityId EntityId)
{
	Worker_EntityId SetLeader = SharedData->ActorSets.GetSetLeader(EntityId);
	if (SetLeader != SpatialConstants::INVALID_ENTITY_ID)
	{
		TOptional<uint32> CandidateWorker = EvaluateDebugComponent(SetLeader);
		const TSet<Worker_EntityId_Key>* ActorSet = SharedData->ActorSets.GetSet(SetLeader);

		if (ensure(ActorSet != nullptr))
		{
			for (auto SetMember : *ActorSet)
			{
				TOptional<uint32> Worker = EvaluateDebugComponent(SetMember);
				if (Worker)
				{
					ensure(!CandidateWorker || Worker.GetValue() == CandidateWorker.GetValue());
					CandidateWorker = Worker;
				}
			}
		}

		if (CandidateWorker)
		{
			return MakeTuple(SetLeader, CandidateWorker.GetValue());
		}
	}
	else
	{
		TOptional<uint32> Worker = EvaluateDebugComponent(EntityId);
		if (Worker)
		{
			return MakeTuple(EntityId, Worker.GetValue());
		}
	}
	return {};
}

void FLegacyLoadBalancing::EvaluateDebugComponent(Worker_EntityId EntityId, FMigrationContext& Ctx)
{
	auto NewAssignment = EvaluateDebugComponentWithSet(EntityId);
	if (NewAssignment)
	{
		ToRefresh.Remove(NewAssignment->Key);
		int32& CurAssignment = Assignment.FindOrAdd(EntityId, -1);
		if (CurAssignment != NewAssignment->Value - 1)
		{
			CurAssignment = NewAssignment->Value - 1;
			Ctx.EntitiesToMigrate.Add(EntityId, Partitions[CurAssignment]);
		}
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
			ToRefresh = ToRefresh.Difference(Ctx.DeletedEntities);
#if !UE_BUILD_SHIPPING
			if (ServerWorkerCustomAssignment->GetModifiedEntities().Contains(WorkerForCustomAssignment))
			{
				for (const auto& Entry : DebugCompStorage->GetObjects())
				{
					EvaluateDebugComponent(Entry.Key, Ctx);
				}
			}
			else if (DebugCompStorage->GetObjects().Num() > 0)
			{
				for (const auto& Entity : DebugCompStorage->GetModifiedEntities())
				{
					EvaluateDebugComponent(Entity, Ctx);
				}
				TSet<Worker_EntityId_Key> ToRefreshCopy = ToRefresh;
				for (const auto& Entity : ToRefreshCopy)
				{
					EvaluateDebugComponent(Entity, Ctx);
				}
			}
#endif //! UE_BUILD_SHIPPING

			for (Worker_EntityId EntityId : ToRefresh)
			{
				if (Ctx.MigratingEntities.Contains(EntityId))
				{
					NotChecked.Add(EntityId);
					continue;
				}

				const ActorGroupMember& Group = GroupStorage->GetObjects().FindChecked(EntityId);
				FLegacyLBContext::Layer& Layer = LBContext.Layers[Group.ActorGroupId];

				const FVector& Position = PositionStorage->GetPositions().FindChecked(EntityId);
				const FVector2D Actor2DLocation(Position);

				int32& CurAssignment = Assignment.FindOrAdd(EntityId, -1);

				if (CurAssignment >= 0 && CurAssignment < Partitions.Num())
				{
					int32 LayerIndex = CurAssignment + 1 - Layer.FirstWorkerId;
					if (ensureAlways(LayerIndex < Layer.Cells.Num()) && Layer.Cells[LayerIndex].Region.IsInside(Actor2DLocation))
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
