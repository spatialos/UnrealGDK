#include "Interop/SpatialStrategySystem.h"
#include "Interop/Connection/SpatialOSWorkerInterface.h"
#include "Schema/ServerWorker.h"
#include "Utils/InterestFactory.h"

DEFINE_LOG_CATEGORY(LogSpatialStrategySystem);

namespace SpatialGDK
{
SpatialStrategySystem::SpatialStrategySystem(const FSubView& InSubView, Worker_EntityId InStrategyWorkerEntityId,
											 SpatialOSWorkerInterface* Connection)
	: SubView(InSubView)
	, StrategyWorkerEntityId(InStrategyWorkerEntityId)
	, StrategyPartitionEntityId(SpatialConstants::INITIAL_STRATEGY_PARTITION_ENTITY_ID)
{

	Worker_CommandRequest ClaimRequest = Worker::CreateClaimPartitionRequest(StrategyPartitionEntityId);
	StrategyWorkerRequest = Connection->SendCommandRequest(StrategyWorkerEntityId, &ClaimRequest, SpatialGDK::RETRY_UNTIL_COMPLETE, {});
}

void SpatialStrategySystem::Advance(SpatialOSWorkerInterface* Connection)
{
	const FSubViewDelta& SubViewDelta = SubView.GetViewDelta();
	for (const EntityDelta& Delta : SubViewDelta.EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			// TODO
		}
		break;
		case EntityDelta::ADD:
		{
			// TODO
		}
		break;
		case EntityDelta::REMOVE:
		case EntityDelta::TEMPORARILY_REMOVED:
		{
			// TODO
		}
		break;
		default:
			break;
		}
	}
}

void SpatialStrategySystem::Flush(SpatialOSWorkerInterface* Connection)
{
	// TODO
}

void SpatialStrategySystem::Destroy(SpatialOSWorkerInterface* Connection) {}

} // namespace SpatialGDK
