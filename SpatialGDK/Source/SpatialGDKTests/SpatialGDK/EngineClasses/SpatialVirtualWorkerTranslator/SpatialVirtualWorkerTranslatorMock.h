// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/AbstractVirtualWorkerTranslator.h"

#include "UObject/Object.h"

#include "SpatialVirtualWorkerTranslatorMock.generated.h"

UCLASS()
class USpatialVirtualWorkerTranslatorMock : public UObject, public AbstractVirtualWorkerTranslator
{
	GENERATED_BODY()
public:

	void Init(VirtualWorkerId inReturnVirtualWorkerId);

	virtual VirtualWorkerId GetLocalVirtualWorkerId() const override;

private:
	VirtualWorkerId ReturnVirtualWorkerId;
}; 
