// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
void FindFirstOpOfType(const TArray<Worker_OpList*>& InOpLists, const Worker_OpType OpType, Worker_Op** OutOp);
void FindFirstOpOfTypeForComponent(const TArray<Worker_OpList*>& InOpLists, const Worker_OpType OpType, const Worker_ComponentId ComponentId, Worker_Op** OutOp);
Worker_ComponentId GetComponentId(const Worker_Op* Op);
} // namespace SpatialGDK
