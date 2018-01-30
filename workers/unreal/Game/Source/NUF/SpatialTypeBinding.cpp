// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTypeBinding.h"
#include "SpatialPackageMapClient.h"

void USpatialTypeBinding::Init(USpatialUpdateInterop* UpdateInterop, UPackageMap* PackageMap)
{
	this->PackageMap = Cast<USpatialPackageMapClient>(PackageMap);
	check(this->PackageMap);
	this->UpdateInterop = UpdateInterop;
}
