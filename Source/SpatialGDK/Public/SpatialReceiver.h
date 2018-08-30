#pragma once

#include "SpatialReceiver.generated.h"

UCLASS()
class USpatialReceiver : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* NetDriver);

	// Dispatcher Interface

private:
};
