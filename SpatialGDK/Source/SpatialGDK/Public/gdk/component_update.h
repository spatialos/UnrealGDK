#ifndef GDK_COMPONENT_UPDATE_H
#define GDK_COMPONENT_UPDATE_H
#include "gdk/common_types.h"
#include <memory>

struct Schema_Object;
struct Schema_ComponentUpdate;
struct Worker_ComponentUpdate;

namespace gdk {

/** An RAII wrapper for component updates. */
class ComponentUpdate {
public:
  /** Takes ownership of update. */
  explicit ComponentUpdate(Schema_ComponentUpdate* update);
  /** Copies the underlying component update.  */
  explicit ComponentUpdate(const Worker_ComponentUpdate& update);
  /** Creates a new component update. */
  explicit ComponentUpdate(ComponentId componentId);

  ~ComponentUpdate() = default;

  // Moveable, not copyable
  ComponentUpdate(const ComponentUpdate& other) = delete;
  ComponentUpdate(ComponentUpdate&& other) noexcept;
  ComponentUpdate& operator=(const ComponentUpdate& other) = delete;
  ComponentUpdate& operator=(ComponentUpdate&& other) noexcept;

  /** Creates a copy of the component update. */
  ComponentUpdate Clone() const;
  /** Releases ownership of the Schema_ComponentUpdate. */
  Schema_ComponentUpdate* Release() &&;

  /** Appends the fields and events from other to the update. */
  bool Merge(ComponentUpdate other);

  Schema_Object* GetFields();
  const Schema_Object* GetFields() const;

  Schema_Object* GetEvents();
  const Schema_Object* GetEvents() const;

  Schema_ComponentUpdate* GetUnderlying() const;

  ComponentId GetComponentId() const;

private:
  struct Deleter {
    void operator()(Schema_ComponentUpdate* update) const noexcept;
  };

  std::unique_ptr<Schema_ComponentUpdate, Deleter> update;
};

}  // namespace gdk
#endif  // GDK_COMPONENT_UPDATE_H
