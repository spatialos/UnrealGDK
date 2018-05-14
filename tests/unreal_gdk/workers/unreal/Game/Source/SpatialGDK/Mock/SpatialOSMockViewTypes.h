#pragma once
#include "SpatialOSMockWorkerTypes.h"
#include "improbable/view.h"

class SPATIALGDK_API MockView : public MockDispatcher
{
public:
  template <typename... T>
  MockView(const worker::Components<T...>& components) : SpatialOSDispatcher{components}
  {
    OnAddEntity([this](const worker::AddEntityOp& op) {
      Entities[op.EntityId];
      ComponentAuthority[op.EntityId];
    });
    OnRemoveEntity([this](const worker::RemoveEntityOp& op) {
      Entities.erase(op.EntityId);
      ComponentAuthority.erase(op.EntityId);
    });
    // ForEachComponent(components, TrackComponentHandler{ *this });
  }

  // Not copyable or movable.
  MockView(const MockView&) = delete;
  MockView(MockView&&) = delete;
  MockView& operator=(const MockView&) = delete;
  MockView& operator=(MockView&&) = delete;

  template <typename T>
  worker::Authority GetAuthority(worker::EntityId entity_id) const
  {
    auto entityIterator = ComponentAuthority.find(entity_id);
    if (entityIterator == ComponentAuthority.end())
    {
      return worker::Authority::kNotAuthoritative;
    }
    auto authorityIterator = entityIterator->second.find(T::ComponentId);
    return authorityIterator == entityIterator->second.end() ? worker::Authority::kNotAuthoritative
                                                             : authorityIterator->second;
  }

  worker::Map<worker::EntityId, worker::Entity> Entities;

  worker::Map<worker::EntityId, worker::Map<worker::ComponentId, worker::Authority>>
      ComponentAuthority;
};

using SpatialOSView = MockView;