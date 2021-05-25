// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/LegacyLoadBalancingStrategy.h"

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/SpatialOSWorkerInterface.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "LoadBalancing/LBDataStorage.h"
#include "LoadBalancing/LoadBalancingCalculator.h"
#include "LoadBalancing/PartitionManager.h"

namespace SpatialGDK
{
FLegacyLoadBalancing::FLegacyLoadBalancing(UAbstractLBStrategy& LegacyLBStrat, SpatialVirtualWorkerTranslator& InTranslator)
	: Translator(InTranslator)
{
	ExpectedWorkers = LegacyLBStrat.GetMinimumRequiredWorkers();
	VirtualWorkerIdToHandle.SetNum(ExpectedWorkers);

	if (!LegacyLBStrat.IsStrategyWorkerAware())
	{
		AssignmentStorage = MakeUnique<FDirectAssignmentStorage>();
	}
	else
	{
		PositionStorage = MakeUnique<SpatialGDK::FSpatialPositionStorage>();
		GroupStorage = MakeUnique<SpatialGDK::FActorGroupStorage>();

		FLegacyLBContext LBContext;
		Calculator = LegacyLBStrat.CreateLoadBalancingCalculator(LBContext);

		if (LBContext.Layers != nullptr)
		{
			LBContext.Layers->SetGroupData(*GroupStorage);
		}
		for (auto* Grid : LBContext.Grid)
		{
			Grid->SetPositionData(*PositionStorage);
		}
	}
}

FLegacyLoadBalancing::~FLegacyLoadBalancing() {}

void FLegacyLoadBalancing::Advance(SpatialOSWorkerInterface* Connection)
{
	const TArray<Worker_Op>& Messages = Connection->GetWorkerMessages();

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
			}
			WorkerTranslationRequest.Reset();
		}
		}
	}
}

void FLegacyLoadBalancing::QueryTranslation(SpatialOSWorkerInterface* Connection)
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

	WorkerTranslationRequest = Connection->SendEntityQueryRequest(&TranslationQuery, RETRY_UNTIL_COMPLETE);
}

void FLegacyLoadBalancing::Flush(SpatialOSWorkerInterface* Connection)
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

		if (Calculator != nullptr)
		{
			Calculator->CollectPartitionsToAdd(PartitionMgr, Partitions);
		}
		else
		{
			for (VirtualWorkerId i = 1; i <= ExpectedWorkers; ++i)
			{
				Partitions.Add(PartitionMgr.CreatePartition(nullptr, QueryConstraint()));
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
		if (Calculator)
		{
			Calculator->CollectEntitiesToMigrate(Ctx);
		}
		else
		{
			const TMap<Worker_EntityId_Key, AuthorityIntent>& AssignmentMap = AssignmentStorage->GetAssignments();
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
