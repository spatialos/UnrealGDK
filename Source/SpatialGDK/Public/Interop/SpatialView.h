#pragma once

#include "CoreMinimal.h"

#include <improbable/c_worker.h>
#include <improbable/c_schema.h>

#include "Schema/UnrealMetadata.h"

#include "SpatialView.generated.h"

class USpatialNetDriver;
class USpatialReceiver;

UCLASS()
class SPATIALGDK_API USpatialView : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* NetDriver);
	void ProcessOps(Worker_OpList* OpList);

	Worker_Authority GetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	UnrealMetadata* GetUnrealMetadata(Worker_EntityId EntityId);

private:
	void OnAddComponent(Worker_AddComponentOp& add_component);
	void OnRemoveComponent(Worker_RemoveComponentOp& remove_component);
	void OnAuthorityChange(const Worker_AuthorityChangeOp& Op);

	USpatialReceiver* Receiver;
	TArray<Worker_Op*> QueuedComponentUpdateOps;

	TMap<Worker_EntityId, TMap<Worker_ComponentId, Worker_Authority>> EntityComponentAuthorityMap;
	TMap<Worker_EntityId, TSharedPtr<UnrealMetadata>> EntityUnrealMetadataMap;
};
