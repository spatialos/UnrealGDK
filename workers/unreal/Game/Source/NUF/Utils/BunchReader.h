// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialHandlePropertyMap.h"
#include "Serialization/BitReader.h"

class FBunchReader
{
public:
	struct ReplicatedData
	{
		UProperty* Property;
		TArray<uint8> Data;
	};

	using RepDataHandler = TFunction<bool(FNetBitReader&, UPackageMap*, int32, UProperty*)>;

	FBunchReader(FNetBitReader* InBunch);

	bool Parse(bool bIsServer, const FRepHandlePropertyMap& PropertyMap, RepDataHandler RepDataHandlerFunc);

	// Access properties.
	bool HasError() const;
	bool HasRepLayout() const;
	bool IsActor() const;

private:
	bool bError;
	bool bHasRepLayout;
	bool bIsActor;

	FNetBitReader* Bunch;

	void ReadHeader(bool bIsServer);
};
