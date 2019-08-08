#ifndef GDK_ENTITY_QUERY_H
#define GDK_ENTITY_QUERY_H
#include "gdk/common_types.h"
#include <WorkerSDK/improbable/c_worker.h>
#include <deque>
#include <vector>

namespace gdk {

class EntityQuery {
public:
  explicit EntityQuery(const Worker_EntityQuery& query);

  ~EntityQuery() = default;

  // Moveable, not copyable
  EntityQuery(const EntityQuery&) = delete;
  EntityQuery(EntityQuery&&) = default;
  EntityQuery& operator=(const EntityQuery&) = delete;
  EntityQuery& operator=(EntityQuery&&) = default;

  const Worker_EntityQuery* GetWorkerQuery() const;

private:
  void StoreConstraint(const Worker_Constraint& constraint);

  Worker_ResultType resultType;
  std::vector<ComponentId> snapshotComponentIds;
  std::deque<Worker_Constraint> constraints;  // Stable pointer storage.
  Worker_EntityQuery workerQuery;             // Storage for the return value.
};

}  // namespace gdk
#endif  // GDK_ENTITY_QUERY_H