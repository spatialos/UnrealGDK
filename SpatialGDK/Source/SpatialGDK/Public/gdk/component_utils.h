#ifndef GDK_COMPONENT_UTILS_H
#define GDK_COMPONENT_UTILS_H
#include "gdk/common_types.h"
#include "gdk/component_data.h"
#include "gdk/component_update.h"
#include <WorkerSDK/improbable/c_schema.h>
#include <cstring>
#include <string>
#include <vector>

namespace gdk {

static const ComponentId kEntityAclComponentId = 50;
static const ComponentId kMetadataComponentId = 53;
static const ComponentId kPositionComponentId = 54;
static const ComponentId kPersistenceComponentId = 55;

inline ComponentData CreatePositionComponent(double x, double y, double z) {
  ComponentData data{kPositionComponentId};
  auto* fields = data.GetFields();
  auto* coords = Schema_AddObject(fields, 1);
  Schema_AddDouble(coords, 1, x);
  Schema_AddDouble(coords, 2, y);
  Schema_AddDouble(coords, 3, z);
  return data;
}

inline ComponentUpdate CreatePositionUpdate(double x, double y, double z) {
  ComponentUpdate update{kPositionComponentId};
  auto* fields = update.GetFields();
  auto* coords = Schema_AddObject(fields, 1);
  Schema_AddDouble(coords, 1, x);
  Schema_AddDouble(coords, 2, y);
  Schema_AddDouble(coords, 3, z);
  return update;
}

inline ComponentUpdate CreatePositionUpdate(const double* x, const double* y, const double* z) {
  ComponentUpdate update{kPositionComponentId};
  auto* fields = update.GetFields();
  auto* coords = Schema_AddObject(fields, 1);
  if (x != nullptr) {
    Schema_AddDouble(coords, 1, *x);
  }
  if (y != nullptr) {
    Schema_AddDouble(coords, 2, *y);
  }
  if (z != nullptr) {
    Schema_AddDouble(coords, 3, *z);
  }
  return update;
}

inline ComponentData CreatePersistenceComponent() {
  ComponentData data{kPersistenceComponentId};
  return data;
}

inline ComponentData CreateMetadataComponent(const std::string& name) {
  ComponentData data{kMetadataComponentId};
  auto* fields = data.GetFields();
  auto* nameBuffer = Schema_AllocateBuffer(fields, static_cast<std::uint32_t>(name.length()));
  std::memcpy(nameBuffer, name.data(), name.length());
  Schema_AddBytes(fields, 1, nameBuffer, static_cast<std::uint32_t>(name.length()));
  return data;
}

}  // namespace gdk
#endif  // GDK_COMPONENT_UTILS_H
