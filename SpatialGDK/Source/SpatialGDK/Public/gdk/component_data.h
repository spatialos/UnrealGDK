#ifndef GDK_COMPONENT_DATA_H
#define GDK_COMPONENT_DATA_H
#include "gdk/common_types.h"
#include <memory>

struct Schema_Object;
struct Schema_ComponentData;
struct Worker_ComponentData;

namespace gdk {

class ComponentUpdate;

/** An RAII wrapper for component data. */
class ComponentData {
public:
  /** Takes ownership of the provided data. */
  explicit ComponentData(Schema_ComponentData* data);
  /** Copies the underlying component data.  */
  explicit ComponentData(const Worker_ComponentData& data);
  /** Creates a new component data. */
  explicit ComponentData(ComponentId componentId);

  ~ComponentData() = default;

  // Moveable, not copyable.
  ComponentData(const ComponentData& other) = delete;
  ComponentData(ComponentData&& other) noexcept;
  ComponentData& operator=(const ComponentData& other) = delete;
  ComponentData& operator=(ComponentData&& other) noexcept;

  /** Creates a copy of the component data. */
  ComponentData Clone() const;
  /** Releases ownership of the component data. */
  Schema_ComponentData* Release() &&;

  /** Appends the fields from the provided update. */
  bool ApplyUpdate(const ComponentUpdate& update);

  Schema_Object* GetFields();
  const Schema_Object* GetFields() const;

  Schema_ComponentData* GetUnderlying();
  const Schema_ComponentData* GetUnderlying() const;

  ComponentId GetComponentId() const;

private:
  struct Deleter {
    void operator()(Schema_ComponentData* data) const noexcept;
  };

  std::unique_ptr<Schema_ComponentData, Deleter> data;
};

}  // namespace gdk
#endif  // GDK_COMPONENT_DATA_H
