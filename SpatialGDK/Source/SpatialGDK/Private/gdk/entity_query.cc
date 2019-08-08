#include "gdk/entity_query.h"

#include <cstdint>

namespace gdk {

EntityQuery::EntityQuery(const Worker_EntityQuery& query)
: resultType(static_cast<Worker_ResultType>(query.result_type))
, snapshotComponentIds(
      resultType == WORKER_RESULT_TYPE_SNAPSHOT ? query.snapshot_result_type_component_id_count : 0)
, workerQuery() {
  StoreConstraint(query.constraint);

  if (resultType == WORKER_RESULT_TYPE_SNAPSHOT) {
    for (std::uint32_t i = 0; i < query.snapshot_result_type_component_id_count; ++i) {
      snapshotComponentIds[i] = query.snapshot_result_type_component_ids[i];
    }
  }

  workerQuery = Worker_EntityQuery{constraints[0], static_cast<std::uint8_t>(resultType),
                                   static_cast<std::uint32_t>(snapshotComponentIds.size()),
                                   snapshotComponentIds.data()};
}

const Worker_EntityQuery* EntityQuery::GetWorkerQuery() const {
  if (workerQuery.constraint.constraint_type == 0) {
    return nullptr;
  }
  return &workerQuery;
}

// Stores the constraints inside a vector.
// Sub-constraints for "and", "or", and "not" constraints will be stored adjacent to their parent.
// The pointers to these constraints are then re-pointed to them.
void EntityQuery::StoreConstraint(const Worker_Constraint& constraint) {
  const auto index = constraints.size();
  constraints.emplace_back(constraint);
  switch (static_cast<Worker_ConstraintType>(constraint.constraint_type)) {
  case WORKER_CONSTRAINT_TYPE_ENTITY_ID:
  case WORKER_CONSTRAINT_TYPE_COMPONENT:
  case WORKER_CONSTRAINT_TYPE_SPHERE:
    break;
  case WORKER_CONSTRAINT_TYPE_AND:
    for (std::uint32_t i = 0; i < constraint.and_constraint.constraint_count; ++i) {
      StoreConstraint(constraint.and_constraint.constraints[i]);
    }
    // Adjust pointers to point at the child constraint.
    constraints[index].and_constraint.constraints = &constraints[index + 1];
    break;
  case WORKER_CONSTRAINT_TYPE_OR:
    for (std::uint32_t i = 0; i < constraint.or_constraint.constraint_count; ++i) {
      StoreConstraint(constraint.or_constraint.constraints[i]);
    }
    // Adjust pointers to point at the child constraint.
    constraints[index].or_constraint.constraints = &constraints[index + 1];
    break;
  case WORKER_CONSTRAINT_TYPE_NOT:
    StoreConstraint(*constraint.not_constraint.constraint);
    // Adjust pointers to point at the child constraint.
    constraints[index].not_constraint.constraint = &constraints[index + 1];
    break;
  }
}

}  // namespace gdk
