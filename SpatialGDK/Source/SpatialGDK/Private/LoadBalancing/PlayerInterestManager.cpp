#include "LoadBalancing/PlayerInterestManager.h"

#include "LoadBalancing/ActorSetSystem.h"
#include "LoadBalancing/EntitySpatialScene.h"
#include "SpatialView/SpatialOSWorker.h"
#include "Utils/InterestFactory.h"

namespace SpatialGDK
{
FPlayerInterestManager::FPlayerInterestManager(FEntitySpatialScene& InSpatialScene, InterestFactory& InInterestF)
	: SpatialScene(InSpatialScene)
	, InterestF(InInterestF)
	, EvaluationTime(0.5 / FPlatformTime::GetSecondsPerCycle64())
{
	NextEvaluationTS = FPlatformTime::Cycles64() + EvaluationTime;
}

void FPlayerInterestManager::AddPlayer(ISpatialOSWorker& Connection, Worker_EntityId EntityId)
{
	TArray<Worker_EntityId> Dummy;
	TSet<Worker_EntityId> DummySet;
	UpdatePlayerInterest(Connection, EntityId, DummySet, Dummy);

	Players.Add(EntityId);
}

void FPlayerInterestManager::UpdatePlayersInterest(ISpatialOSWorker& Connection, const FActorSetSystem& ActorSets)
{
	// Do that per player instead.
	if (FPlatformTime::Cycles64() < NextEvaluationTS)
	{
		return;
	}

	SpatialScene.UpdateNeighbours(30000.0);
	for (auto const& PlayerInfo : SpatialScene.GetNeighbourInfo())
	{
		TArrayView<const Worker_EntityId> HiResNeigh(&PlayerInfo.Neighbours[0], PlayerInfo.NumNeighbours);
		TSet<Worker_EntityId> DummySet;
		const TSet<Worker_EntityId>* PCSet = ActorSets.GetSet(PlayerInfo.EntityId);
		UpdatePlayerInterest(Connection, PlayerInfo.EntityId, PCSet ? *PCSet : DummySet, HiResNeigh);
	}
	NextEvaluationTS = FPlatformTime::Cycles64() + EvaluationTime;
}

void FPlayerInterestManager::UpdatePlayerInterest(ISpatialOSWorker& Connection, Worker_EntityId EntityId,
												  const TSet<Worker_EntityId>& ActorSet, TArrayView<const Worker_EntityId> HiResEntities)
{
	Interest ResultInterest;

	InterestF.AddClientSelfInterest(ResultInterest);
	InterestF.AddServerSelfInterest(ResultInterest);

	QueryConstraint AlwaysRelevantConstraint = InterestF.CreateClientAlwaysRelevantConstraint();

	Query ClientSystemQuery;
	ClientSystemQuery.Constraint = AlwaysRelevantConstraint;
	ClientSystemQuery.ResultComponentIds = InterestF.GetClientNonAuthInterestResultType().ComponentIds;
	ClientSystemQuery.ResultComponentSetIds = InterestF.GetClientNonAuthInterestResultType().ComponentSetsIds;

	InterestF.AddComponentQueryPairToInterestComponent(ResultInterest, SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID, ClientSystemQuery);

	for (const auto& CheckoutRadiusConstraintFrequencyPair : InterestF.GetClientCheckoutRadiusConstraint())
	{
		if (!CheckoutRadiusConstraintFrequencyPair.Constraint.IsValid())
		{
			continue;
		}

		Query NewQuery;

		NewQuery.Constraint.AndConstraint.Add(CheckoutRadiusConstraintFrequencyPair.Constraint);

		// Make sure that the Entity is not marked as bHidden
		QueryConstraint VisibilityConstraint;
		VisibilityConstraint = InterestF.CreateActorVisibilityConstraint();
		NewQuery.Constraint.AndConstraint.Add(VisibilityConstraint);

		NewQuery.ResultComponentIds = { SpatialConstants::POSITION_COMPONENT_ID, SpatialConstants::UNREAL_METADATA_COMPONENT_ID };

		InterestF.AddComponentQueryPairToInterestComponent(ResultInterest, SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID, NewQuery);
	}

	QueryConstraint EntityIdListConstraint;
	for (auto Entity : ActorSet)
	{
		QueryConstraint EntityIdConstraint;
		EntityIdConstraint.EntityIdConstraint = Entity;
		EntityIdListConstraint.OrConstraint.Add(EntityIdConstraint);
	}
	for (auto Entity : HiResEntities)
	{
		QueryConstraint EntityIdConstraint;
		EntityIdConstraint.EntityIdConstraint = Entity;
		EntityIdListConstraint.OrConstraint.Add(EntityIdConstraint);
	}

	Query NewQuery;
	// NewQuery.Constraint = EntityIdListConstraint;
	NewQuery.Constraint.AndConstraint.Add(EntityIdListConstraint);

	// Make sure that the Entity is not marked as bHidden
	QueryConstraint VisibilityConstraint;
	VisibilityConstraint = InterestF.CreateActorVisibilityConstraint();
	NewQuery.Constraint.AndConstraint.Add(VisibilityConstraint);

	NewQuery.ResultComponentIds = InterestF.GetClientNonAuthInterestResultType().ComponentIds;
	NewQuery.ResultComponentSetIds = InterestF.GetClientNonAuthInterestResultType().ComponentSetsIds;

	InterestF.AddComponentQueryPairToInterestComponent(ResultInterest, SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID, NewQuery);

	Worker_ComponentUpdate UpdateData = ResultInterest.CreateInterestUpdate();
	ComponentUpdate Update(OwningComponentUpdatePtr(UpdateData.schema_type), Interest::ComponentId);
	Connection.SendComponentUpdate(EntityId, MoveTemp(Update));
}
} // namespace SpatialGDK
