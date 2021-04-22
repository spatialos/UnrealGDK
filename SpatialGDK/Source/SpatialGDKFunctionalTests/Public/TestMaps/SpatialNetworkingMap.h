// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "SpatialNetworkingMap.generated.h"

/**
 * This map is mostly for native-spatial compatibility tests, so things that work in native that should also work under our networking.
 * If you can, please add your test to this map, the more tests we have in a map, the more efficient it is to run them (no need for a new
 * session / deployment for every test). That does mean that tests here should ideally create everything they need dynamically, and clean up
 * everything after themselves, so other tests can run with a clean slate.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialNetworkingMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatialNetworkingMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
