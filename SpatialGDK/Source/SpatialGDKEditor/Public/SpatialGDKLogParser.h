// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

class FSpatialGDKLogParser
{
public:
	FSpatialGDKLogParser();

	~FSpatialGDKLogParser();

private:
	TSharedPtr<class FMissingSchemaLogParser> MissingSchemaErrorParser;

	FDelegateHandle PreBeginPIEDelegateHandle;
	FDelegateHandle EndPIEDelegateHandle;
};
