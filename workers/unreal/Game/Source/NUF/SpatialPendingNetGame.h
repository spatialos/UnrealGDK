// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/PendingNetGame.h"
#include "SpatialPendingNetGame.generated.h"

class UEngine;
struct FWorldContext;

//NUF-sourcechange UPendingNetGame needs to have its dllexport defined: "class ENGINE_API UPendingNetGame". This can count as a bug, we can submit a PR.
UCLASS(transient)
class USpatialPendingNetGame : public UPendingNetGame
{
	GENERATED_UCLASS_BODY()
		
	//NUF-sourcechange Made virtual in PendingNetGame.h
	virtual void InitNetDriver() override;
};
