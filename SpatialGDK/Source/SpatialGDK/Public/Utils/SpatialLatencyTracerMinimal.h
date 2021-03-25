// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

namespace worker
{
namespace c
{
struct Schema_Object;
}
}

class FSpatialLatencyTracerMinimal
{
public:
	static int32 ReadTraceFromSchemaObject(worker::c::Schema_Object* Obj, uint32 FieldId);
	static void WriteTraceToSchemaObject(int32 Key, worker::c::Schema_Object* Obj, uint32 FieldId);
};
