
#include <improbable/c_worker.h>
#include <improbable/c_schema.h>

#include "ConnectionConfig.h"

#include "SpatialConnection.generated.h"

UCLASS()
class USpatialConnection : public UObject
{
public:
	void Init();

	void ConnectToSpatial(ReceptionistConfig Config);
	void ConnectToSpatial(LocatorConfig Config);
	void GetDeploymentList(Worker_Locator Locator);

private:
	Worker_Connection* Connection;
};
