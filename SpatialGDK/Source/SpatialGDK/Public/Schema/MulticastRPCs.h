// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialConstants.h"
#include "Utils/RPCRingBuffer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
struct MulticastRPCs
{
	MulticastRPCs(Schema_ComponentData* Data);

	void ApplyComponentUpdate(Schema_ComponentUpdate* Update);

	RPCRingBuffer MulticastRPCBuffer;
	uint32 InitiallyPresentMulticastRPCsCount = 0;

private:
	void ReadFromSchema(Schema_Object* SchemaObject);
};

class MulticastRPCsLayout : RPCComponentLayout
{
public:
	MulticastRPCsLayout();

	virtual RPCRingBufferDescriptor GetRingBufferDescriptor(ERPCType Type) const override;

	void MoveLastSentIdToInitiallyPresentCount(Schema_Object* SchemaObject, uint64 LastSentId);

private:
	RPCRingBufferDescriptor MulticastRPCBufferDescriptor;
	Schema_FieldId InitiallyPresentMulticastRPCsCountFieldId;
};

} // namespace SpatialGDK
