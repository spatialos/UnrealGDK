// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialFunctionalTestAutoDestroyComponent.h"

USpatialFunctionalTestAutoDestroyComponent::USpatialFunctionalTestAutoDestroyComponent()
	: Super()
{
#if ENGINE_MINOR_VERSION <= 23
	bReplicates = true;
#else
	SetIsReplicatedByDefault(true);
#endif
}
