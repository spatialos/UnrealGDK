// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialPartitionSystem.h"
#include "Interop/SpatialPartitionSystemImpl.h"
#include "Interop/Startup/PartitionCreationStartupSteps.h"
#include "LoadBalancing/PartitionManagerImpl.h"
#include "SpatialGDKSettings.h"
#include "SpatialView/EntityComponentTypes.h"

namespace SpatialGDK
{
struct FPartitionMgrStartupSharedState : TSharedFromThis<FPartitionMgrStartupSharedState>
{
	FPartitionMgrStartupSharedState(ISpatialOSWorker& InConnection, FPartitionManager::Impl& InPartitionMgr)
		: Connection(InConnection)
		, PartitionMgr(InPartitionMgr)
		, PartitionDiscoveryState(MakeShared<FPartitionCreationSharedState>())
	{
	}

	ISpatialOSWorker& Connection;
	FPartitionManager::Impl& PartitionMgr;
	TSharedPtr<FPartitionCreationSharedState> PartitionDiscoveryState;
};

class FReservePartitionIdRange : public FStartupStep
{
public:
	FReservePartitionIdRange(TSharedRef<FPartitionMgrStartupSharedState> InData)
		: Data(InData)
	{
		StepName = TEXT("Reserve ParitionId Range and claim strategy worker partition");
	}

	virtual void Start()
	{
		StrategyWorkerRequest = Data->PartitionMgr.CommandsHandler.ClaimPartition(
			Data->Connection, Data->PartitionMgr.StrategyWorkerEntityId, SpatialConstants::INITIAL_STRATEGY_PARTITION_ENTITY_ID,
			[this](const Worker_CommandResponseOp& Op) {
				if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
				{
					UE_LOG(LogSpatialPartitionManager, Error, TEXT("Claim Strategy partition failed : %s"), UTF8_TO_TCHAR(Op.message));
				}
				bStrategyPartitionClaimed = true;
			});

		Data->PartitionMgr.WaitForPartitionIdAvailable(Data->Connection);
	}

	virtual bool TryFinish()
	{
		Data->PartitionMgr.CommandsHandler.ProcessOps(Data->Connection.GetWorkerMessages());
		return bStrategyPartitionClaimed && !Data->PartitionMgr.WaitForPartitionIdAvailable(Data->Connection);
	}

protected:
	TSharedRef<FPartitionMgrStartupSharedState> Data;
	Worker_RequestId StrategyWorkerRequest;
	bool bStrategyPartitionClaimed = false;
};

class FFillPartitionManagerWithDiscoveredPartitions : public FStartupStep
{
public:
	FFillPartitionManagerWithDiscoveredPartitions(TSharedRef<FPartitionMgrStartupSharedState> InData)
		: Data(InData)
	{
		StepName = TEXT("Fill Manager with discovered Partitions");
	}

	virtual void Start()
	{
		ensure(Data->PartitionDiscoveryState->DiscoveredPartitionEntityIds.Num() == 0);
		// TODO
		// Restoring and exposing the custom data on partition would also be needed for the load balancer to pick it up.
		// Leaving it incomplete for now since recovering snapshot is not as urgent as before.
		for (const auto& RecoveredPartition : Data->PartitionDiscoveryState->DiscoveredPartitionEntityIds)
		{
			FPartitionHandle NewPartition = MakeShared<FPartitionDesc>();
			NewPartition->State = MakeUnique<FPartitionInternalState>();
			NewPartition->State->bPartitionCreated = true;

			if (const ComponentData* MetadataComp =
					RecoveredPartition.Value.Components.FindByPredicate(ComponentIdEquality({ SpatialConstants::METADATA_COMPONENT_ID })))
			{
				Worker_ComponentData dummy;
				dummy.schema_type = MetadataComp->GetUnderlying();
				Metadata DeserializedComp(dummy);
				NewPartition->State->DisplayName = DeserializedComp.EntityType;
			}

			if (const ComponentData* InterestComp =
					RecoveredPartition.Value.Components.FindByPredicate(ComponentIdEquality({ SpatialConstants::INTEREST_COMPONENT_ID })))
			{
				Worker_ComponentData dummy;
				dummy.schema_type = InterestComp->GetUnderlying();
				Interest DeserializedComp(dummy);
				if (ComponentSetInterest* InterestSet =
						DeserializedComp.ComponentInterestMap.Find(SpatialConstants::PARTITION_WORKER_AUTH_COMPONENT_SET_ID))
				{
					if (InterestSet->Queries.Num() > 0)
					{
						// As seen in InterestFactory::CreatePartitionInterest
						NewPartition->State->LBConstraint = InterestSet->Queries[0].Constraint;
					}
				}
			}

			for (const auto& Component : RecoveredPartition.Value.Components)
			{
				if (Component.GetComponentId() != SpatialConstants::METADATA_COMPONENT_ID
					&& Component.GetComponentId() != SpatialConstants::INTEREST_COMPONENT_ID)
				{
					NewPartition->State->CurrentMetadataComponents.Add(Component.GetComponentId());
				}
			}

			NewPartition->State->Id = RecoveredPartition.Key;
			Data->PartitionMgr.Partitions.Add(NewPartition);
		}
	}

	virtual bool TryFinish() { return true; }

protected:
	TSharedRef<FPartitionMgrStartupSharedState> Data;
};

TArray<TUniquePtr<FStartupStep>> CreatePartitionManagerStartupSequence(ISpatialOSWorker& Connection, FPartitionManager::Impl& PartitionMgr)
{
	TArray<TUniquePtr<FStartupStep>> Steps;
	TSharedPtr<FPartitionMgrStartupSharedState> StartupData = MakeShared<FPartitionMgrStartupSharedState>(Connection, PartitionMgr);

	TArray<Worker_ComponentId> ComponentsToRead({ SpatialConstants::METADATA_COMPONENT_ID, SpatialConstants::INTEREST_COMPONENT_ID });

	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();
	if (UClass* PartitionSystemClass = Settings->PartitionSystemClass.Get())
	{
		USpatialPartitionSystem* PartitionSystem = Cast<USpatialPartitionSystem>(PartitionSystemClass->GetDefaultObject());
		TArray<FLBDataStorage*> ComponentData = PartitionSystem->GetData();
		for (auto Storage : ComponentData)
		{
			for (auto ComponentId : Storage->GetComponentsToWatch())
			{
				ComponentsToRead.AddUnique(ComponentId);
			}
		}
	}

	Steps.Add(MakeUnique<FReservePartitionIdRange>(StartupData.ToSharedRef()));
	Steps.Add(MakeUnique<FDiscoverExistingPartitionsStep>(StartupData->PartitionDiscoveryState.ToSharedRef(), Connection,
														  SpatialConstants::LOADBALANCER_PARTITION_TAG_COMPONENT_ID, ComponentsToRead));
	Steps.Add(MakeUnique<FFillPartitionManagerWithDiscoveredPartitions>(StartupData.ToSharedRef()));

	return Steps;
}
} // namespace SpatialGDK
