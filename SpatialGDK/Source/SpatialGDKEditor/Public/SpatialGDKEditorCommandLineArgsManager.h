// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Templates/SharedPointer.h"

using ILauncherRef = TSharedRef<class ILauncher>;
using ILauncherWorkerPtr = TSharedPtr<class ILauncherWorker>;
using ILauncherProfileRef = TSharedRef<class ILauncherProfile>;

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
	void OnCreateLauncher(ILauncherRef LauncherRef);
	void OnLaunch(ILauncherWorkerPtr LauncherWorkerPtr, ILauncherProfileRef LauncherProfileRef);
	void OnLauncherCanceled(double ExecutionTime);
	void OnLauncherFinished(bool bSuccess, double ExecutionTime, int32 ReturnCode);

	void RemoveCommandLineFromDevice();

	static bool TryConstructMobileCommandLineArgumentsFile(FString& OutCommandLineArgsFile);
	static bool TryPushCommandLineArgsToDevice(const FString& Executable, const FString& ExeArguments, const FString& CommandLineArgsFile);

private:
	bool bAndroidDevice;
};
