// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialConstants.h"
#include "Utils/RPCRingBuffer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
class ClientEndpointLayout : RPCComponentLayout
{
public:
	ClientEndpointLayout();

	virtual RPCRingBufferDescriptor GetRingBufferDescriptor(ERPCType Type) const override;
	virtual Schema_FieldId GetAckFieldId(ERPCType Type) const override;

private:
	RPCRingBufferDescriptor ServerReliableRPCBufferDescriptor;
	RPCRingBufferDescriptor ServerUnreliableRPCBufferDescriptor;
	RPCRingBufferDescriptor MovementRPCBufferDescriptor;
	Schema_FieldId ClientReliableRPCAckFieldId;
	Schema_FieldId ClientUnreliableRPCAckFieldId;
};

struct ClientEndpoint
{
	ClientEndpoint(Schema_ComponentData* Data);

	void ApplyComponentUpdate(Schema_ComponentUpdate* Update);

	RPCRingBuffer ServerReliableRPCBuffer;
	RPCRingBuffer ServerUnreliableRPCBuffer;
	RPCRingBuffer MovementRPCBuffer;
	uint64 ClientReliableRPCAck = 0;
	uint64 ClientUnreliableRPCAck = 0;

private:
	void ReadFromSchema(Schema_Object* SchemaObject);
};

} // namespace SpatialGDK
