// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

class ASpatialFunctionalTest;

class SettingsOverrideHelper
{
public:
	SettingsOverrideHelper();

	static void VerifyNumberOfClients(int32 ExpectedNumberOfClients, ASpatialFunctionalTest* SpatialFunctionalTest);
};
