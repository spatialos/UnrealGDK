// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/ConnectionConfig.h"

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

#include "SpatialWorkerConnection.generated.h"

DECLARE_DELEGATE(OnConnectedDelegate)

UCLASS()
class USpatialWorkerConnection : public UObject
{

	GENERATED_BODY()

public:
	virtual void FinishDestroy() override;

	void Connect(bool bConnectAsClient);

	bool IsConnected();

	// Worker Connection Interface
	Worker_OpList* GetOpList();
	Worker_RequestId SendReserveEntityIdRequest();
	Worker_RequestId SendCreateEntityRequest(uint32_t ComponentCount, const Worker_ComponentData* Components, const Worker_EntityId* EntityId);
	Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId);
	void SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate);
	Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId);
	void SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response);
	void SendLogMessage(const uint8_t Level, const char* LoggerName, const char* Message);

	OnConnectedDelegate OnConnected;

private:
	void ConnectToReceptionist(bool bConnectAsClient);
	void ConnectToLocator();

	Worker_ConnectionParameters CreateConnectionParameters(FConnectionConfig& Config);
	bool ShouldConnectWithLocator();

private:
	Worker_Connection* WorkerConnection;
	Worker_Locator* WorkerLocator;

	FReceptionistConfig ReceptionistConfig;
	FLocatorConfig LocatorConfig;

	bool bIsConnected;
};
