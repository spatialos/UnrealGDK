#include "Schema/ActorGroupMember.h"
#include "Schema/ActorOwnership.h"
#include "Schema/ActorSetMember.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/DebugComponent.h"
#include "Schema/GameplayDebuggerComponent.h"
#include "Schema/Interest.h"
#include "Schema/MigrationDiagnostic.h"
#include "Schema/NetOwningClientWorker.h"
#include "Schema/PlayerSpawner.h"
#include "Schema/Restricted.h"
#include "Schema/ServerWorker.h"
#include "Schema/SnapshotVersionComponent.h"
#include "Schema/SpatialDebugging.h"
#include "Schema/SpawnData.h"
#include "Schema/StandardLibrary.h"
#include "Schema/Tombstone.h"
#include "Schema/UnrealMetadata.h"

#if __cplusplus <= 201402L
namespace SpatialGDK
{
constexpr Worker_ComponentId ActorOwnership::ComponentId;
constexpr Worker_ComponentId ActorGroupMember::ComponentId;
constexpr Worker_ComponentId ActorSetMember::ComponentId;
constexpr Worker_ComponentId DebugComponent::ComponentId;
constexpr Worker_ComponentId AuthorityIntent::ComponentId;
constexpr Worker_ComponentId AuthorityIntentACK::ComponentId;
constexpr Worker_ComponentId GameplayDebuggerComponent::ComponentId;
constexpr Worker_ComponentId Interest::ComponentId;
constexpr Worker_ComponentId MigrationDiagnostic::ComponentId;
constexpr Worker_ComponentId NetOwningClientWorker::ComponentId;
constexpr Worker_ComponentId PlayerSpawner::ComponentId;
constexpr Worker_ComponentId Partition::ComponentId;
constexpr Worker_ComponentId ServerWorker::ComponentId;
constexpr Worker_ComponentId SnapshotVersion::ComponentId;
constexpr Worker_ComponentId SpatialDebugging::ComponentId;
constexpr Worker_ComponentId SpawnData::ComponentId;
constexpr Worker_ComponentId Metadata::ComponentId;
constexpr Worker_ComponentId Position::ComponentId;
constexpr Worker_ComponentId Persistence::ComponentId;
constexpr Worker_ComponentId Worker::ComponentId;
constexpr Worker_ComponentId AuthorityDelegation::ComponentId;
constexpr Worker_ComponentId Tombstone::ComponentId;
constexpr Worker_ComponentId UnrealMetadata::ComponentId;
} // namespace SpatialGDK
#endif
