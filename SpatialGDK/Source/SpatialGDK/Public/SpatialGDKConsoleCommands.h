// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "SpatialCommonTypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKConsoleCommands, Log, All);

class UWorld;

namespace SpatialGDKConsoleCommands
{
void ConsoleCommand_ConnectToLocator(const TArray<FString>& Args, UWorld* World);
}
// namespace
