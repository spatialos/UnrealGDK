#pragma once

#include "CoreMinimal.h"

#include <improbable/c_worker.h>
#include <improbable/c_schema.h>

#include "SpatialView.generated.h"

class USpatialNetDriver;

UCLASS()
class SPATIALGDK_API USpatialView : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* NetDriver);

	void ProcessOps(Worker_OpList* OpList);

	Worker_Authority GetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId);

private:
	void OnAuthorityChange(const Worker_AuthorityChangeOp& Op);

	class USpatialReceiver* Receiver;

	TMap<Worker_EntityId, TMap<Worker_ComponentId, Worker_Authority>> EntityComponentAuthorityMap;
};
