// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "EngineClasses/SpatialNetConnection.h"

namespace improbable
{

FORCEINLINE FString GetOwnerWorkerAttribute(AActor* Actor)
{
	FString NetOwnerAttribute;
	if (const USpatialNetConnection* NetConnection = Cast<USpatialNetConnection>(Actor->GetNetConnection()))
	{
		NetOwnerAttribute = NetConnection->WorkerAttribute;
	}

	return NetOwnerAttribute;
}

}
