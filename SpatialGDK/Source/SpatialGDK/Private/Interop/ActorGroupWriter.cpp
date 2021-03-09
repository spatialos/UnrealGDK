// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/ActorGroupWriter.h"

#include "LoadBalancing/AbstractLBStrategy.h"

namespace SpatialGDK
{
ActorGroupMember ActorGroupWriter::GetActorGroupData(AActor* Actor) const
{
	return ActorGroupMember(LoadBalancingStrategy.GetActorGroupId(*Actor));
}
} // namespace SpatialGDK
