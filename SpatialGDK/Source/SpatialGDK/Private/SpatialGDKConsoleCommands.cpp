// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKConsoleCommands.h"

#include "SpatialConstants.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKConsoleCommands)

namespace SpatialGDKConsoleCommands
{
	void ConsoleCommand_ConnectToLocator(const TArray<FString>& Args, UWorld* World)
	{
		if (Args.Num() != 2)
		{
			UE_LOG(LogSpatialGDKConsoleCommands, Log, TEXT("ConsoleCommand_ConnectToLocator takes 2 arguments (login, playerToken). Only %d given."), Args.Num());
			return;
		}

		FURL URL;
		URL.Host = SpatialConstants::LOCATOR_HOST;
		FString Login = SpatialConstants::URL_LOGIN_OPTION + Args[0];
		FString PlayerIdentity = SpatialConstants::URL_PLAYER_IDENTITY_OPTION + Args[1];
		URL.AddOption(*PlayerIdentity);
		URL.AddOption(*Login);

		FString Error;
		FWorldContext &WorldContext = GEngine->GetWorldContextFromWorldChecked(World);
		GEngine->Browse(WorldContext, URL, Error);
	}

	FAutoConsoleCommandWithWorldAndArgs ConnectToLocatorCommand = FAutoConsoleCommandWithWorldAndArgs(
		TEXT("ConnectToLocator"),
		TEXT("Usage: ConnectToLocator <login> <playerToken>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&ConsoleCommand_ConnectToLocator)
	);
}
