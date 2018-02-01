// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTypeBinding.h"
#include "SpatialPackageMapClient.h"

void USpatialTypeBinding::Init(USpatialUpdateInterop* InUpdateInterop, USpatialPackageMapClient* InPackageMap)
{
	check(InUpdateInterop);
	check(InPackageMap);
	UpdateInterop = InUpdateInterop;
	PackageMap = InPackageMap;
}
