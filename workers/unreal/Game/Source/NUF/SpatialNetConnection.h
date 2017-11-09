// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IpConnection.h"
#include "SpatialNetConnection.generated.h"

/**
 * 
 */
UCLASS(transient, config = Engine)
class NUF_API USpatialNetConnection : public UIpConnection
{
	GENERATED_BODY()
};
