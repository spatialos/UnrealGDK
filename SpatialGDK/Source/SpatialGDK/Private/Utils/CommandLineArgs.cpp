// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/CommandLineArgs.h"

DEFINE_LOG_CATEGORY(LogSpatialCommandLineArgs);

void SpatialGDK::CheckCmdLineOverrideBool(const TCHAR* CommandLine, const TCHAR* Parameter, const TCHAR* PrettyName, bool& bOutValue)
{
#if ALLOW_SPATIAL_CMDLINE_PARSING // Command-line only enabled for non-shipping or with target rule bEnableSpatialCmdlineInShipping enabled
	if (FParse::Param(CommandLine, Parameter))
	{
		bOutValue = true;
	}
	else
	{
		TCHAR TempStr[16];
		if (FParse::Value(CommandLine, Parameter, TempStr, 16) && TempStr[0] == '=')
		{
			bOutValue = FCString::ToBool(TempStr + 1); // + 1 to skip =
		}
	}
#endif // ALLOW_SPATIAL_CMDLINE_PARSING
	UE_LOG(LogSpatialCommandLineArgs, Log, TEXT("%s is %s."), PrettyName, bOutValue ? TEXT("enabled") : TEXT("disabled"));
}

void SpatialGDK::CheckCmdLineOverrideOptionalBool(const TCHAR* CommandLine, const TCHAR* Parameter, const TCHAR* PrettyName,
												  TOptional<bool>& bOutValue)
{
#if ALLOW_SPATIAL_CMDLINE_PARSING // Command-line only enabled for non-shipping or with target rule bEnableSpatialCmdlineInShipping enabled
	if (FParse::Param(CommandLine, Parameter))
	{
		bOutValue = true;
	}
	else
	{
		TCHAR TempStr[16];
		if (FParse::Value(CommandLine, Parameter, TempStr, 16) && TempStr[0] == '=')
		{
			bOutValue = FCString::ToBool(TempStr + 1); // + 1 to skip =
		}
	}
#endif // ALLOW_SPATIAL_CMDLINE_PARSING
	UE_LOG(LogSpatialCommandLineArgs, Log, TEXT("%s is %s."), PrettyName,
		   bOutValue.IsSet() ? bOutValue ? TEXT("enabled") : TEXT("disabled") : TEXT("not set"));
}

void SpatialGDK::CheckCmdLineOverrideOptionalString(const TCHAR* CommandLine, const TCHAR* Parameter, const TCHAR* PrettyName,
													TOptional<FString>& StrOutValue)
{
#if ALLOW_SPATIAL_CMDLINE_PARSING
	FString TempStr;
	if (FParse::Value(CommandLine, Parameter, TempStr) && TempStr[0] == '=')
	{
		StrOutValue = TempStr.Right(TempStr.Len() - 1); // + 1 to skip =
	}
#endif // ALLOW_SPATIAL_CMDLINE_PARSING
	UE_LOG(LogSpatialCommandLineArgs, Log, TEXT("%s is %s."), PrettyName,
		   StrOutValue.IsSet() ? *(StrOutValue.GetValue()) : TEXT("not set"));
}
