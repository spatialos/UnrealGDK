#include "LoadBalancing/CustomLoadBalancingStrategy.h"

#include "Interop/SkeletonEntityManifestPublisher.h"
#include "Interop/Startup/SpatialStartupCommon.h"
#include "LoadBalancing/PartitionManager.h"
#include "PartitionManagerImpl.h"
#include "Schema/Interest.h"

namespace SpatialGDK
{
constexpr int32 EntityCountInBucket = 2;

static int64 GetEntityBucketId(const Worker_EntityId EntityId)
{
	if (EntityId < SpatialConstants::FIRST_AVAILABLE_ENTITY_ID)
	{
		return 42;
	}
	return (EntityId + 1000) / EntityCountInBucket;
}

PRAGMA_DISABLE_OPTIMIZATION

int64 FCustomLoadBalancingStrategy::GetBucketId(FPartitionHandle Partition) const
{
	return Algo::FindByPredicate(PartitionBucketIndexToPartitionHandle,
								 [Partition](const TPair<int64, FPartitionHandle>& Kvp) {
									 return Kvp.Value == Partition;
								 })
		->Key;
}

FCustomLoadBalancingStrategy::FCustomLoadBalancingStrategy(FTicker& Ticker)
{
	PositionStorage = MakeUnique<FSpatialPositionStorage>();

	FDelegateHandle Handle = Ticker.AddTicker(FTickerDelegate::CreateLambda([this](float) {
												  AssignedBucketPartitions.Reset();
												  return true;
											  }),
											  10.0f);
	OnDestroyed = [this, &Ticker, Handle] {
		Ticker.RemoveTicker(Handle);
	};
}

FCustomLoadBalancingStrategy::~FCustomLoadBalancingStrategy()
{
	OnDestroyed();
}

void FCustomLoadBalancingStrategy::Init(ISpatialOSWorker& Connection, FLoadBalancingSharedData InSharedData,
										TArray<FLBDataStorage*>& OutLoadBalancingData, TArray<FLBDataStorage*>& OutServerWorkerData)
{
	OutLoadBalancingData.Emplace(PositionStorage.Get());
	SharedData = InSharedData;

	SkeletonLoadBalancing.Emplace(InSharedData, [this](FPartitionHandle Partition) {
		return GetBucketId(Partition);
	});

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

	TArray<TUniquePtr<FStartupStep>> StartupSteps;
	StartupSteps.Emplace(MakeUnique<FGenericStartupStep>("Wait For Partition Manager", FGenericStartupStep::StartFn(), [this] {
		return SharedData->PartitionManager.IsReady();
	}));

	StartupSteps.Emplace(MakeUnique<FGenericStartupStep>("Wait For Expected Number of Workers", FGenericStartupStep::StartFn(), [this] {
		return ConnectedWorkers.Num() > 0;
	}));

	StartupSteps.Emplace(MakeUnique<FGenericStartupStep>(
		"Assign GSM authority",
		[this, &Connection]() {
			CommandsHandler.ClaimPartition(Connection, ConnectedWorkers[0]->State->SystemWorkerId,
										   SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
		},
		FGenericStartupStep::TryFinishFn()));

	Executor.Emplace(MoveTemp(StartupSteps));
}

void FCustomLoadBalancingStrategy::CollectEntitiesToMigrate(FMigrationContext& Ctx)
{
	TSet<Worker_EntityId_Key> NotChecked;

	ToRefresh = ToRefresh.Union(Ctx.ModifiedEntities);
	ToRefresh = ToRefresh.Difference(Ctx.DeletedEntities);

	for (const Worker_EntityId_Key EntityId : ToRefresh)
	{
		if (Ctx.MigratingEntities.Contains(EntityId))
		{
			NotChecked.Add(EntityId);
			continue;
		}

		OnEntityAdded(EntityId);

		FPartitionHandle TargetPartition = FindEntityPartition(EntityId);
		const int32* CurrentBucket = Assignments.Find(EntityId);
		if (CurrentBucket == nullptr || GetBucketId(TargetPartition) != *CurrentBucket)
		{
			Ctx.EntitiesToMigrate.Emplace(EntityId, TargetPartition);
			Assignments.Emplace(EntityId, GetBucketId(TargetPartition));
		}
	}

	SkeletonLoadBalancing->ApplySkeletonsToMigrate(Ctx);

	ToRefresh = MoveTemp(NotChecked);
}

PRAGMA_ENABLE_OPTIMIZATION

void FCustomLoadBalancingStrategy::OnEntityAdded(Worker_EntityId EntityId)
{
	FPartitionManager& PartitionManager = SharedData->PartitionManager;
	const int64 EntityBucketId = GetEntityBucketId(EntityId);
	if (PartitionBucketIndexToPartitionHandle.Contains(EntityBucketId))
	{
		return;
	}
	auto Partition = PartitionManager.CreatePartition(FString::Printf(TEXT("BucketPartition-%lld"), GetEntityBucketId(EntityId)), {}, {});
	PartitionBucketIndexToPartitionHandle.Emplace(GetEntityBucketId(EntityId), Partition);
}

FPartitionHandle FCustomLoadBalancingStrategy::FindEntityPartition(const Worker_EntityId EntityId) const
{
	const int64 EntityBucketIndex = GetEntityBucketId(EntityId);
	return PartitionBucketIndexToPartitionHandle.FindChecked(EntityBucketIndex);
}

void FCustomLoadBalancingStrategy::OnWorkersConnected(TArrayView<FLBWorkerHandle> InConnectedWorkers)
{
	for (FLBWorkerHandle ConnectedWorker : InConnectedWorkers)
	{
		ConnectedWorkers.Emplace(ConnectedWorker);
		FPartitionHandle Partition = WorkerPartitions.Emplace(
			ConnectedWorker, SharedData->PartitionManager.CreatePartition(
								 FString::Printf(TEXT("WorkerPartition-%s"), *ConnectedWorker->State->FullWorkerName), {}, {}));
		SharedData->PartitionManager.AssignPartitionTo(Partition, ConnectedWorker);
	}
}

void FCustomLoadBalancingStrategy::Advance(ISpatialOSWorker& Connection)
{
	CommandsHandler.ProcessOps(Connection.GetWorkerMessages());
	if (Executor)
	{
		Executor->TryFinish();
	}
}

bool FCustomLoadBalancingStrategy::IsReady()
{
	return Executor && Executor->HasFinished();
}

void FCustomLoadBalancingStrategy::TickPartitions()
{
	if (ConnectedWorkers.Num() < 2)
	{
		return;
	}
	for (auto BucketPartitions : PartitionBucketIndexToPartitionHandle)
	{
		if (AssignedBucketPartitions.Contains(BucketPartitions.Value))
		{
			continue;
		}
		AssignedBucketPartitions.Emplace(BucketPartitions.Value);
		const int8 BucketPartitionWorker = NextPartitionAssignmentWorkerIndex % ConnectedWorkers.Num();
		NextPartitionAssignmentWorkerIndex = (NextPartitionAssignmentWorkerIndex + 1) % ConnectedWorkers.Num();
		SharedData->PartitionManager.AssignPartitionTo(BucketPartitions.Value, ConnectedWorkers[BucketPartitionWorker]);
	}
}

void FCustomLoadBalancingStrategy::OnSkeletonManifestReceived(Worker_EntityId EntityId, FSkeletonEntityManifest Manifest)
{
	SkeletonLoadBalancing->OnSkeletonManifestReceived(EntityId, Manifest, Assignments);
}

void FCustomLoadBalancingStrategy::Flush(ISpatialOSWorker& Connection)
{
	if (WorkerPartitions.Num() == 0)
	{
		return;
	}
	TArray<FPartitionHandle> PartitionHandles;
	PartitionBucketIndexToPartitionHandle.GenerateValueArray(PartitionHandles);
	TMap<int32, Worker_PartitionId> PartitionIds;
	bool bAllPartitionsComplete = true;
	Algo::Transform(PartitionHandles, PartitionIds,
					[this, &bAllPartitionsComplete](FPartitionHandle Partition) -> TPair<int32, Worker_PartitionId> {
						auto PartitionId = SharedData->PartitionManager.GetPartitionId(Partition);
						if (PartitionId)
						{
							return TPair<int32, Worker_PartitionId>{ GetBucketId(Partition), *PartitionId };
						}
						bAllPartitionsComplete = false;
						return {};
					});

	if (bAllPartitionsComplete)
	{
		SkeletonLoadBalancing->Flush(Connection, PartitionIds);
	}
}
} // namespace SpatialGDK
