// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/ConnectionConfig.h"

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

#include "SpatialConnection.generated.h"

DECLARE_DELEGATE(OnConnectedDelegate)

UCLASS()
class USpatialConnection : public UObject
{

	GENERATED_BODY()

public:
	virtual void FinishDestroy() override;

	void Init();

	void Connect(ReceptionistConfig Config);
	void Connect(LocatorConfig Config);

	bool IsConnected();

	Worker_OpList* GetOpList(uint32_t TimeoutMillis);

	Worker_RequestId SendReserveEntityIdRequest(const uint32_t* TimeoutMillis);
	Worker_RequestId SendCreateEntityRequest(uint32_t ComponentCount, const Worker_ComponentData* Components, const Worker_EntityId* EntityId, const uint32_t* TimeoutMillis);
	Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId, const uint32_t* TimeoutMillis);

	void SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate);
	Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId, const uint32_t* TimeoutMillis, const Worker_CommandParameters* CommandParameters);
	void SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response);

	OnConnectedDelegate OnConnected;

	Worker_Connection* Connection;
	bool bIsConnected;

private:
	Worker_ConnectionParameters CreateConnectionParameters(ConnectionConfig& Config);

private:
	Worker_Locator* Locator;
};
