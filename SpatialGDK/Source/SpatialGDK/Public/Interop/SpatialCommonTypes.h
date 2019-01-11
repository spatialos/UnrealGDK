// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_worker.h>

using FChannelObjectPair = TPair<TWeakObjectPtr<class USpatialActorChannel>, TWeakObjectPtr<UObject>>;
struct FObjectReferences;
using FObjectReferencesMap = TMap<int32, FObjectReferences>;
using FReliableRPCMap = TMap<Worker_RequestId, TSharedRef<struct FPendingRPCParams>>;
