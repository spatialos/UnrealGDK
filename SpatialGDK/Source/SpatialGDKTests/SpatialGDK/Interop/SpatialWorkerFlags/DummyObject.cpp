// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DummyObject.h"

void  UDummyObject::SetFlagUpdated(const FString& flagName, const FString& flagValue)
{
	isFlagUpdated = true;
	timesUpdated++;

	return;
}

