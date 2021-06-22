#pragma once

#include "EngineClasses/SpatialNetDriver.h"
#include "Schema/AuthorityIntent.h"
#include "Utils/ComponentReader.h"

namespace SpatialGDK
{
class FLBDataStorage
{
public:
	virtual ~FLBDataStorage() = default;

	virtual void OnAdded(Worker_EntityId EntityId, const SpatialGDK::EntityViewElement& Element) = 0;
	virtual void OnRemoved(Worker_EntityId EntityId) = 0;
	virtual void OnUpdate(Worker_EntityId EntityId, Worker_ComponentId InComponentId, Schema_ComponentUpdate* Update) = 0;

	void ClearModified() { Modified.Empty(); }

	const TSet<Worker_EntityId_Key>& GetModifiedEntities() const { return Modified; }

	const TSet<Worker_ComponentId>& GetComponentsToWatch() const { return Components; }

protected:
	TSet<Worker_EntityId_Key> Modified;
	TSet<Worker_ComponentId> Components;
};

class FSpatialPositionStorage : public FLBDataStorage
{
public:
	FSpatialPositionStorage();

	virtual void OnAdded(Worker_EntityId EntityId, const SpatialGDK::EntityViewElement& Element) override;
	virtual void OnRemoved(Worker_EntityId EntityId) override;
	virtual void OnUpdate(Worker_EntityId EntityId, Worker_ComponentId InComponentId, Schema_ComponentUpdate* Update) override;

	TMap<Worker_EntityId_Key, FVector> const& GetPositions() const { return Positions; }

protected:
	TMap<Worker_EntityId_Key, FVector> Positions;
};

class FActorGroupStorage : public FLBDataStorage
{
public:
	FActorGroupStorage();

	virtual void OnAdded(Worker_EntityId EntityId, const SpatialGDK::EntityViewElement& Element) override;
	virtual void OnRemoved(Worker_EntityId EntityId) override;
	virtual void OnUpdate(Worker_EntityId EntityId, Worker_ComponentId InComponentId, Schema_ComponentUpdate* Update) override;

	TMap<Worker_EntityId_Key, int32> const& GetGroups() const { return Groups; }

protected:
	TMap<Worker_EntityId_Key, int32> Groups;
};

class FDirectAssignmentStorage : public FLBDataStorage
{
public:
	FDirectAssignmentStorage();

	virtual void OnAdded(Worker_EntityId EntityId, const SpatialGDK::EntityViewElement& Element) override;
	virtual void OnRemoved(Worker_EntityId EntityId) override;
	virtual void OnUpdate(Worker_EntityId EntityId, Worker_ComponentId InComponentId, Schema_ComponentUpdate* Update) override;

	TMap<Worker_EntityId_Key, AuthorityIntent> const& GetAssignments() const { return Intents; }

protected:
	TMap<Worker_EntityId_Key, AuthorityIntent> Intents;
};
} // namespace SpatialGDK
