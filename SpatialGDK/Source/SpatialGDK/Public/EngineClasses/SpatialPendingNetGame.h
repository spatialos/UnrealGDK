// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/PendingNetGame.h"
#include "UObject/ObjectMacros.h"

#include "SpatialPendingNetGame.generated.h"

// UPendingNetGame needs to have its dllexport defined: "class ENGINE_API UPendingNetGame". This can count as a bug, we can submit a PR.
UCLASS(transient)
class USpatialPendingNetGame : public UPendingNetGame
{
	GENERATED_UCLASS_BODY()

	// Made virtual in PendingNetGame.h
	virtual void InitNetDriver() override;

	virtual void SendJoin() override;
};
