// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTypeBinding.h"
#include "SpatialPackageMapClient.h"

void USpatialTypeBinding::Init(USpatialInterop* InInterop, USpatialPackageMapClient* InPackageMap)
{
	check(InInterop);
	check(InPackageMap);
	Interop = InInterop;
	PackageMap = InPackageMap;
	bIsSingleton = false;
}
