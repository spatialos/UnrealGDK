// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "RelevancyTestActors.h"

AAlwaysRelevantTestActor::AAlwaysRelevantTestActor()
{
	bAlwaysRelevant = true;
	NetCullDistanceSquared = 0;
}

AAlwaysRelevantServerOnlyTestActor::AAlwaysRelevantServerOnlyTestActor()
{
	bAlwaysRelevant = true;
	NetCullDistanceSquared = 0;
}

AOnlyRelevantToOwnerTestActor::AOnlyRelevantToOwnerTestActor()
{
	bOnlyRelevantToOwner = true;
	NetCullDistanceSquared = 0;
}

AUseOwnerRelevancyTestActor::AUseOwnerRelevancyTestActor()
{
	bNetUseOwnerRelevancy = true;
	NetCullDistanceSquared = 0;
}
