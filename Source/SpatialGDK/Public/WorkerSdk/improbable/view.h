// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#ifndef IMPROBABLE_WORKER_SDK_INCLUDE_IMPROBABLE_VIEW_H
#define IMPROBABLE_WORKER_SDK_INCLUDE_IMPROBABLE_VIEW_H
#include <improbable/worker.h>

namespace worker {

/**
 * The View is an optional data structure that maintains the known state of entities in the worker's
 * view of the simulation. This object should not be modified concurrently by multiple threads. In
 * particular, a call to Process() can modify the Entities map as a side-effect, and so should not
 * be made concurrently with any manual modification of the Entities map.
 *
 * Note that as of SpatialOS 12 this class is intended primarily as an example: using the
 * worker::ForEachComponent functionality, a custom View can be implemented from scratch with any
 * semantics desired.
 */
class View : public Dispatcher {
public:
  template <typename... T>
  View(const Components<T...>& components) : Dispatcher{components} {
    OnAddEntity([this](const AddEntityOp& op) {
      Entities[op.EntityId];
      ComponentAuthority[op.EntityId];
    });
    OnRemoveEntity([this](const RemoveEntityOp& op) {
      Entities.erase(op.EntityId);
      ComponentAuthority.erase(op.EntityId);
    });
    ForEachComponent(components, TrackComponentHandler{*this});
  }

  // Not copyable or movable.
  View(const View&) = delete;
  View(View&&) = delete;
  View& operator=(const View&) = delete;
  View& operator=(View&&) = delete;

  /**
   * Helper function that checks if the worker has authority over a particular component of a
   * particular entity. The template parameter T should be a generated component metaclass.
   */
  template <typename T>
  Authority GetAuthority(EntityId entity_id) const {
    auto it = ComponentAuthority.find(entity_id);
    if (it == ComponentAuthority.end()) {
      return Authority::kNotAuthoritative;
    }
    auto jt = it->second.find(T::ComponentId);
    return jt == it->second.end() ? Authority::kNotAuthoritative : jt->second;
  }

  /** Current component data for all entities in the worker's view. */
  Map<EntityId, Entity> Entities;
  /** Current authority delegations. */
  Map<EntityId, Map<ComponentId, worker::Authority>> ComponentAuthority;

private:
  struct TrackComponentHandler {
    View& view_reference;

    template <typename T>
    void Accept() const {
      View& view = this->view_reference;

      view.OnAddComponent<T>([&view](const AddComponentOp<T>& op) {
        auto it = view.Entities.find(op.EntityId);
        if (it != view.Entities.end()) {
          Entity& entity = it->second;
          entity.Add<T>(op.Data);
          view.ComponentAuthority[op.EntityId][T::ComponentId] = Authority::kNotAuthoritative;
        }
      });

      view.OnRemoveComponent<T>([&view](const RemoveComponentOp& op) {
        auto it = view.Entities.find(op.EntityId);
        if (it != view.Entities.end()) {
          Entity& entity = it->second;
          entity.Remove<T>();
        }

        auto jt = view.ComponentAuthority.find(op.EntityId);
        if (jt != view.ComponentAuthority.end()) {
          jt->second.erase(T::ComponentId);
        }
      });

      view.OnAuthorityChange<T>([&view](const AuthorityChangeOp& op) {
        view.ComponentAuthority[op.EntityId][T::ComponentId] = op.Authority;
      });

      view.OnComponentUpdate<T>([&view](const ComponentUpdateOp<T>& op) {
        auto it = view.Entities.find(op.EntityId);
        if (it != view.Entities.end()) {
          Entity& entity = it->second;
          if (entity.Get<T>()) {
            entity.Update<T>(op.Update);
          }
        }
      });
    }
  };
};

}  // ::worker

#endif  // IMPROBABLE_WORKER_SDK_INCLUDE_IMPROBABLE_VIEW_H
