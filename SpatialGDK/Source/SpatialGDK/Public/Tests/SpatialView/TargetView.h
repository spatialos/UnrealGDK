// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/EntityView.h"
#include "SpatialView/MessagesToSend.h"
#include "SpatialView/OpList/EntityComponentOpList.h"

namespace SpatialGDK
{
/**
 * Generates OpLists to describe changes to a View.
 * No further changes can be made after calling Disconnect.
 * Functions that add things to the view will add the entity if it does not already exist.
 * Functions that remove or modify things require that thing to have previously existed.
 * Functions that add command requests and responses have no entity prerequisites.
 */
class FTargetView
{
public:
	/**
	 * Adds an entity to the view.
	 * Requires the entity to not be in the view.
	 */
	void AddEntity(Worker_EntityId EntityId);

	/**
	 * Removes an entity from the view.
	 * Requires the entity to be in the view.
	 * Removes present authority and components.
	 */
	void RemoveEntity(Worker_EntityId EntityId);

	/**
	 * Add component to the view, or set all values if the component already exists.
	 * Adds the entity to the view if it does not exist.
	 */
	void AddOrSetComponent(Worker_EntityId EntityId, ComponentData Data);

	/**
	 * Applies an update to an entity-component in the view.
	 * Requires the entity-component to be in the view.
	 */
	void UpdateComponent(Worker_EntityId EntityId, ComponentUpdate Update);

	/**
	 * Removes an entity-component from the view.
	 * Requires the entity-component to be in the view.
	 */
	void RemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId);

	/**
	 * Adds authority over an entity-component-set to the view.
	 * Adds the entity to the view if it does not exist.
	 */
	void AddAuthority(Worker_EntityId EntityId, Worker_ComponentSetId ComponentSetId);

	/**
	 * Removes authority over an entity-component-set to the view.
	 * Requires authority over an entity-component-set.
	 */
	void RemoveAuthority(Worker_EntityId EntityId, Worker_ComponentSetId ComponentSetId);

	/**
	 * Sets the view as disconnected. No other further changes can be made after calling this.
	 */
	void Disconnect(Worker_ConnectionStatusCode StatusCode, StringStorage DisconnectReason);

	void AddCreateEntityCommandResponse(Worker_EntityId EntityId, Worker_RequestId RequestId, Worker_StatusCode StatusCode,
										StringStorage Message);
	void AddEntityQueryCommandResponse(Worker_RequestId RequestId, TArray<OpListEntity> Results, Worker_StatusCode StatusCode,
									   StringStorage Message);
	void AddEntityCommandRequest(Worker_EntityId EntityId, Worker_RequestId RequestId, CommandRequest CommandRequest);
	void AddEntityCommandResponse(Worker_EntityId EntityId, Worker_RequestId RequestId, Worker_StatusCode StatusCode,
								  StringStorage Message);
	void AddDeleteEntityCommandResponse(Worker_EntityId EntityId, Worker_RequestId RequestId, Worker_StatusCode StatusCode,
										StringStorage Message);
	void AddReserveEntityIdsCommandResponse(Worker_EntityId EntityId, uint32 NumberOfEntities, Worker_RequestId RequestId,
											Worker_StatusCode StatusCode, StringStorage Message);

	/** Get the current state of the view. */
	const EntityView& GetView() const;

	/** Create an OpList representing the changes made to the view since the last call. */
	OpList CreateOpListFromChanges();

private:
	EntityView View;
	EntityComponentOpListBuilder Builder;
	bool bDisconnected = false;
};

} // namespace SpatialGDK
