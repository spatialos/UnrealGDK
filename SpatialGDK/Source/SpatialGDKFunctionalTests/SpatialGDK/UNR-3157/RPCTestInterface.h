// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Engine/Classes/Engine/EngineTypes.h"
#include "UObject/Interface.h"
#include "RPCTestInterface.generated.h"

UINTERFACE(Blueprintable)
class URPCTestInterface : public UInterface
{
	GENERATED_BODY()
};

class IRPCTestInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(Client, Reliable)
	virtual void RPCInInterface();
};
