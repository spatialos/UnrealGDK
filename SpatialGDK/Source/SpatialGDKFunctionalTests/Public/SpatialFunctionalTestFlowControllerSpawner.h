#pragma once
#include "Math/Transform.h"
#include "Templates/SubclassOf.h"

class APlayerController;
class ASpatialFunctionalTestFlowController;
class ASpatialFunctionalTest;
class UWorld;

class SpatialFunctionalTestFlowControllerSpawner
{
public:
	// default constructor has to exist for generated code, shouldn't be used in user code
	SpatialFunctionalTestFlowControllerSpawner();
	SpatialFunctionalTestFlowControllerSpawner(ASpatialFunctionalTest* ControllerOwningTest,
											   TSubclassOf<ASpatialFunctionalTestFlowController> FlowControllerClassToSpawn);

	void ModifyFlowControllerClassToSpawn(TSubclassOf<ASpatialFunctionalTestFlowController> FlowControllerClassToSpawn);

	ASpatialFunctionalTestFlowController* SpawnServerFlowController();
	ASpatialFunctionalTestFlowController* SpawnClientFlowController(APlayerController* OwningClient);

	void AssignClientFlowControllerId(ASpatialFunctionalTestFlowController* ClientFlowController);

private:
	ASpatialFunctionalTest* OwningTest;
	TSubclassOf<ASpatialFunctionalTestFlowController> FlowControllerClass;
	uint8 NextClientControllerId;

	uint8 OwningServerIntanceId(UWorld* World) const;
	void LockFlowControllerDelegations(ASpatialFunctionalTestFlowController* FlowController) const;
};
