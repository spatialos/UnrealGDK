// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/LegacyLoadBalancingStrategy.h"

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/SpatialOSWorkerInterface.h"
#include "Interop/SkeletonEntityManifestPublisher.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "LoadBalancing/ActorSetSystem.h"
#include "LoadBalancing/LBDataStorage.h"
#include "LoadBalancing/LegacyLoadBalancingCommon.h"
#include "LoadBalancing/LegacyLoadbalancingComponents.h"
#include "LoadBalancing/PartitionManager.h"
#include "Schema/DebugComponent.h"

DEFINE_LOG_CATEGORY(LogSpatialLegacyLoadBalancing)

namespace SpatialGDK
{
class FGenericStartupStep : public FStartupStep
{
public:
	using StartFn = TFunction<void()>;
	using TryFinishFn = TFunction<bool()>;

	FGenericStartupStep(FString Name, StartFn InOnStart, TryFinishFn InOnTryFinish)
		: OnStart(InOnStart)
		, OnTryFinish(InOnTryFinish)
	{
		StepName = Name;
	}

	virtual void Start() override
	{
		if (OnStart)
		{
			OnStart();
		}
	};
	virtual bool TryFinish()
	{
		if (!OnTryFinish)
		{
			return true;
		}
		return OnTryFinish();
	}

	StartFn OnStart;
	TryFinishFn OnTryFinish;
};

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

FLegacyLoadBalancing::FLegacyLoadBalancing(UAbstractLBStrategy& LegacyLBStrat, SpatialVirtualWorkerTranslator& InTranslator)
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
}

FLegacyLoadBalancing::~FLegacyLoadBalancing() {}

void FLegacyLoadBalancing::Advance(ISpatialOSWorker& Connection)
{
	CommandsHandler.ProcessOps(Connection.GetWorkerMessages());
	if (StartupExecutor)
	{
		if (!StartupExecutor->TryFinish())
		{
			return;
		}
		StartupExecutor.Reset();
	}
}

void FLegacyLoadBalancing::Flush(ISpatialOSWorker& Connection)
{
	
}

void FLegacyLoadBalancing::Init(ISpatialOSWorker& Connection, FLoadBalancingSharedData InSharedData,
								TArray<FLBDataStorage*>& OutLoadBalancingData, TArray<FLBDataStorage*>& OutServerWorkerData)
{
	SharedData.Emplace(InSharedData);

	TArray<TUniquePtr<FStartupStep>> StartupSteps;
	StartupSteps.Emplace(MakeUnique<FGenericStartupStep>("Wait For Partition Manager", FGenericStartupStep::StartFn(), [this] {
		return SharedData->PartitionManager.IsReady();
	}));

	StartupSteps.Emplace(MakeUnique<FGenericStartupStep>("Wait For Expected Number of Workers", FGenericStartupStep::StartFn(), [this] {
		return ExpectedWorkers == ConnectedWorkers.Num();
	}));

	StartupSteps.Emplace(MakeUnique<FGenericStartupStep>(
		"Assign GSM authority",
		[this, &Connection]() {
			Worker_EntityId MinServerWorkerId = INT64_MAX;
			for (auto WorkerHandle : ConnectedWorkers)
			{
				Worker_EntityId ServerWorkerId = SharedData->PartitionManager.GetSystemWorkerEntityIdForWorker(WorkerHandle);
				if (MinServerWorkerId > ServerWorkerId)
				{
					MinServerWorkerId = ServerWorkerId;
				}
			}

			CommandsHandler.ClaimPartition(Connection, MinServerWorkerId, SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
		},
		FGenericStartupStep::TryFinishFn()));
	StartupSteps.Emplace(MakeUnique<FGenericStartupStep>(
		"Create and assign partitions",
		[this]() {
			CreateAndAssignPartitions();
		},
		FGenericStartupStep::TryFinishFn()));

	StartupExecutor.Emplace(MoveTemp(StartupSteps));

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
	if (!IsReady())
	{
		return;
	}

	FPartitionManager& PartitionMgr = SharedData->PartitionManager;

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

				if (!ensureAlways(LayerIndex < Layer.Cells.Num()) || Layer.Cells[LayerIndex].Region.IsInside(Actor2DLocation))
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
} // namespace SpatialGDK
