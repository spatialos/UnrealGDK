#pragma once

#include "CoreMinimal.h"

#include <improbable/c_worker.h>
#include <improbable/c_schema.h>

#include "SpatialView.generated.h"

UCLASS()
class SPATIALGDK_API USpatialView : public UObject
{
	GENERATED_BODY()

public:
	void Init();

	void ProcessOps(Worker_OpList* OpList);

	Worker_Authority GetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId);

private:
	void OnAuthority(const Worker_Authority& Op);

	class USpatialReceiver* EntityPipeline;
	class USpatialReceiver* Receiver;

	TMap<Worker_EntityId, TMap<Worker_ComponentId, Worker_Authority>> ComponentAuthorityMap;
};
