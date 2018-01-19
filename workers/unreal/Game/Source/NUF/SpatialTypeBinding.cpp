// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTypeBinding.h"

void USpatialTypeBinding::Init(USpatialUpdateInterop* UpdateInterop, UPackageMap* PackageMap)
{
	this->PackageMap = PackageMap;
	this->UpdateInterop = UpdateInterop;
}
