// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/OpList/OpList.h"

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
Worker_Op* FindFirstOpOfType(const TArray<OpList>& InOpLists, const Worker_OpType OpType);
void AppendAllOpsOfType(const TArray<OpList>& InOpLists, const Worker_OpType OpType, TArray<Worker_Op*>& FoundOps);
Worker_Op* FindFirstOpOfTypeForComponent(const TArray<SpatialGDK::OpList>& InOpLists, const Worker_OpType OpType,
										 const Worker_ComponentId ComponentId);
Worker_ComponentId GetComponentId(const Worker_Op* Op);

} // namespace SpatialGDK
