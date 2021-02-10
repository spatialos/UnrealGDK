// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialConstants.h"
#include "Utils/RPCRingBuffer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
struct ServerEndpoint
{
	ServerEndpoint(Schema_ComponentData* Data);

	void ApplyComponentUpdate(Schema_ComponentUpdate* Update);

	RPCRingBuffer ClientReliableRPCBuffer;
	RPCRingBuffer ClientUnreliableRPCBuffer;
	uint64 ServerReliableRPCAck = 0;
	uint64 ServerUnreliableRPCAck = 0;
	uint64 MovementRPCAck = 0;

private:
	void ReadFromSchema(Schema_Object* SchemaObject);
};

class ServerEndpointLayout : RPCComponentLayout
{
public:
	ServerEndpointLayout();

	virtual RPCRingBufferDescriptor GetRingBufferDescriptor(ERPCType Type) const override;
	virtual Schema_FieldId GetAckFieldId(ERPCType Type) const override;

private:
	RPCRingBufferDescriptor ClientReliableRPCBufferDescriptor;
	RPCRingBufferDescriptor ClientUnreliableRPCBufferDescriptor;
	Schema_FieldId ServerReliableRPCAckFieldId;
	Schema_FieldId ServerUnreliableRPCAckFieldId;
	Schema_FieldId MovementRPCAckFieldId;
};

} // namespace SpatialGDK
