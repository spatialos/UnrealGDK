// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialImposterSystemImpl.h"
#include "SpatialView/EntityComponentTypes.h"

namespace SpatialGDK
{
void FImposterSystemImpl::Advance()
{
	ImposterData.Advance();

	for (Worker_EntityId EntityAdded : ImposterData.EntitiesAdded)
	{
		SpatialGDK::FEntityEvent Event = { EntityAdded, SpatialGDK::FEntityEvent::Created };
		Events.Add(Event);
	}

	for (const EntityDelta& Delta : ImposterData.SubView.GetViewDelta().EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::ADD:
		{
			const EntityViewElement* Element = ImposterData.SubView.GetView().Find(Delta.EntityId);
			if (!ensure(Element != nullptr))
			{
				break;
			}
			if (Element->Components.FindByPredicate(ComponentIdEquality({ SpatialConstants::ACTOR_TAG_COMPONENT_ID })))
			{
				SpatialGDK::FEntityEvent Event = { Delta.EntityId, SpatialGDK::FEntityEvent::ToHiRes };
				Events.Add(Event);
			}
		}
		break;

		case EntityDelta::UPDATE:
			for (const auto& Added : Delta.ComponentsAdded)
			{
				if (Added.ComponentId == SpatialConstants::ACTOR_TAG_COMPONENT_ID)
				{
					SpatialGDK::FEntityEvent Event = { Delta.EntityId, SpatialGDK::FEntityEvent::ToHiRes };
					Events.Add(Event);
				}
			}

			for (const auto& Removed : Delta.ComponentsRemoved)
			{
				if (Removed.ComponentId == SpatialConstants::ACTOR_TAG_COMPONENT_ID)
				{
					SpatialGDK::FEntityEvent Event = { Delta.EntityId, SpatialGDK::FEntityEvent::ToLowRes };
					Events.Add(Event);
				}
			}
			break;
		}
	}

	for (Worker_EntityId EntityRemoved : ImposterData.EntitiesRemoved)
	{
		SpatialGDK::FEntityEvent Event = { EntityRemoved, SpatialGDK::FEntityEvent::Deleted };
		Events.Add(Event);
	}
	ImposterData.EntitiesAdded.Empty();
	ImposterData.EntitiesRemoved.Empty();
}

} // namespace SpatialGDK
