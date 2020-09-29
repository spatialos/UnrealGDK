// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Internationalization/Text.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Logging/LogMacros.h"
#include "Templates/SubclassOf.h"
#include "UObject/TextProperty.h"

#include "SpatialGDKSettings.h"

#include "CommandLineArgs.generated.h"

// This log category will always log to the spatial runtime and thus also be printed in the SpatialOutput.
DECLARE_LOG_CATEGORY_EXTERN(LogSpatialCommandLineArgs, Log, All);

namespace SpatialGDK
{
	void CheckCmdLineOverrideBool(const TCHAR* CommandLine, const TCHAR* Parameter, const TCHAR* PrettyName, bool& bOutValue);
	void CheckCmdLineOverrideOptionalBool(const TCHAR* CommandLine, const TCHAR* Parameter, const TCHAR* PrettyName, TOptional<bool>& bOutValue);

	void CheckCmdLineOverrideOptionalString(const TCHAR* CommandLine, const TCHAR* Parameter, const TCHAR* PrettyName,
		TOptional<FString>& StrOutValue);
}
