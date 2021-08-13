#pragma once

#include "LoadBalancing/LBDataStorage.h"
#include "SpatialCommonTypes.h"

namespace SpatialGDK
{
class FEntitySpatialScene;
class InterestFactory;
class ISpatialOSWorker;

struct FPlayerControllerComp
{
	static constexpr Worker_ComponentId ComponentId = SpatialConstants::PLAYER_CONTROLLER_COMPONENT_ID;

	FPlayerControllerComp() {}

	FPlayerControllerComp(const ComponentData& Data)
		: FPlayerControllerComp(Data.GetUnderlying())
	{
	}

	FPlayerControllerComp(Schema_ComponentData* Data)
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

	void ReadFromObject(Schema_Object* Object) {}

	void WriteToObject(Schema_Object* Object) const {}
};

class FPlayerControllerData : public TLBDataStorage<FPlayerControllerComp>
{
};

class FActorSetSystem;
class FPlayerInterestManager
{
public:
	FPlayerInterestManager(FEntitySpatialScene& InSpatialScene, InterestFactory& InInterestF);

	void AddPlayer(ISpatialOSWorker&, Worker_EntityId);

	void UpdatePlayersInterest(ISpatialOSWorker&, const FActorSetSystem& ActorSets);

private:
	void UpdatePlayerInterest(ISpatialOSWorker&, Worker_EntityId, const TSet<Worker_EntityId>&, TArrayView<const Worker_EntityId>);

	uint64 NextEvaluationTS = 0;
	const uint64 EvaluationTime = 0;
	TSet<Worker_EntityId_Key> Players;
	FEntitySpatialScene& SpatialScene;
	InterestFactory& InterestF;
};
} // namespace SpatialGDK
