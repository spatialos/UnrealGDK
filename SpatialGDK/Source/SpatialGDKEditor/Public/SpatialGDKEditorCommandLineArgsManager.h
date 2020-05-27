// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Templates/SharedPointer.h"

using ILauncherRef = TSharedRef<class ILauncher>;
using ILauncherWorkerPtr = TSharedPtr<class ILauncherWorker>;
using ILauncherProfileRef = TSharedRef<class ILauncherProfile>;

#if ENGINE_MINOR_VERSION >= 24
#define ENABLE_LAUNCHER_DELEGATE
#endif

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKEditorCommandLineArgsManager, Log, All);

class FSpatialGDKEditorCommandLineArgsManager
{
public:
	FSpatialGDKEditorCommandLineArgsManager();

	void Init();

	static FReply GenerateDevAuthToken();
	static FReply PushCommandLineToIOSDevice();
	static FReply PushCommandLineToAndroidDevice();
	static FReply RemoveCommandLineFromAndroidDevice();

private:
#ifdef ENABLE_LAUNCHER_DELEGATE
	void OnCreateLauncher(ILauncherRef LauncherRef);
	void OnLaunch(ILauncherWorkerPtr LauncherWorkerPtr, ILauncherProfileRef LauncherProfileRef);
	void OnLauncherCanceled(double ExecutionTime);
	void OnLauncherFinished(bool Outcome, double ExecutionTime, int32 ReturnCode);

	void RemoveCommandLineFromDevice();
#endif // ENABLE_LAUNCHER_DELEGATE

	static bool TryConstructMobileCommandLineArgumentsFile(FString& CommandLineArgsFile);
	static bool TryPushCommandLineArgsToDevice(const FString& Executable, const FString& ExeArguments, const FString& CommandLineArgsFile);

private:
#ifdef ENABLE_LAUNCHER_DELEGATE
	bool bAndroidDevice;
#endif // ENABLE_LAUNCHER_DELEGATE
};
