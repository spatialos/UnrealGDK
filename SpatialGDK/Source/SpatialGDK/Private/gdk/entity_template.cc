#include "gdk/entity_template.h"

#include "gdk/component_utils.h"
#include <WorkerSDK/improbable/c_schema.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace gdk {

EntityTemplate EntityTemplate::Clone() const {
  EntityTemplate t;
  t.entityState = entityState.Clone();
  t.idToAttribute = idToAttribute;
  t.readAttributes = readAttributes;
  return t;
}

EntityState EntityTemplate::CreateEntityState(const std::string& metadata) const {
  auto state = entityState.Clone();
  state.AddComponent(CreateMetadataComponent(metadata));
  state.AddComponent(CreateAcl());
  return state;
}

EntityState EntityTemplate::CreateEntityState() const {
  auto state = entityState.Clone();
  state.AddComponent(CreateAcl());
  return state;
}

void EntityTemplate::AddReadAttribute(const std::string& attribute) {
  readAttributes.emplace_back(attribute);
}

void EntityTemplate::AddComponent(ComponentData&& data, const std::string& layer) {
  idToAttribute.emplace_back(std::make_pair(data.GetComponentId(), layer));
  entityState.AddComponent(std::move(data));
}

void EntityTemplate::UpdateComponent(const ComponentUpdate& update) {
  entityState.TryApplyComponentUpdate(update);
}

void EntityTemplate::RemoveComponent(ComponentId id) {
  entityState.TryRemoveComponent(id);
  for (size_t i = 0; i < idToAttribute.size(); ++i) {
    if (idToAttribute[i].first == id) {
      idToAttribute.erase(idToAttribute.begin() + i);
      return;
    }
  }
}

ComponentData EntityTemplate::CreateAcl() const {
  ComponentData acl{kEntityAclComponentId};
  auto* aclFields = acl.GetFields();
  auto* readAcl = Schema_AddObject(aclFields, 1);
  auto* readAttributeSet = Schema_AddObject(readAcl, 1);

  for (const auto& attribute : readAttributes) {
    std::uint8_t* attributeBuffer =
        Schema_AllocateBuffer(readAttributeSet, static_cast<std::uint32_t>(attribute.length()));
    std::memcpy(attributeBuffer, attribute.data(), attribute.length());
    Schema_AddBytes(readAttributeSet, 1, attributeBuffer,
                    static_cast<std::uint32_t>(attribute.length()));
  }

  auto* writeAcl = Schema_AddObject(aclFields, 2);

  for (const auto& idAndAttribute : idToAttribute) {
    const ComponentId id = idAndAttribute.first;
    const std::string& layer = idAndAttribute.second;
    if (layer.empty()) {
      continue;
    }
    Schema_AddUint32(writeAcl, SCHEMA_MAP_KEY_FIELD_ID, id);
    auto* writeRequirementSet = Schema_AddObject(writeAcl, SCHEMA_MAP_VALUE_FIELD_ID);
    auto* writeAttributesSet = Schema_AddObject(writeRequirementSet, 1);
    std::uint8_t* layerBuffer =
        Schema_AllocateBuffer(readAttributeSet, static_cast<std::uint32_t>(layer.length()));
    std::memcpy(layerBuffer, layer.data(), layer.length());
    Schema_AddBytes(writeAttributesSet, 1, layerBuffer, static_cast<std::uint32_t>(layer.length()));
  }

  return acl;
}

}  // namespace gdk
