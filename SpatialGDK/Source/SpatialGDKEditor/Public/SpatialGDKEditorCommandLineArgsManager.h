// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Templates/SharedPointer.h"

class FReply;

typedef TSharedRef<class ILauncher> ILauncherRef;
typedef TSharedPtr<class ILauncherWorker> ILauncherWorkerPtr;
typedef TSharedRef<class ILauncherProfile> ILauncherProfileRef;

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 24
#define ENABLE_LAUNCHER_DELEGATE
#endif

class FSpatialGDKEditorCommandLineArgsManager
{
public:
	FSpatialGDKEditorCommandLineArgsManager();

	void Startup();

	static FReply GenerateDevAuthToken();
	static FReply PushToIOSDevice();
	static FReply PushToAndroidDevice();
	static FReply RemoveFromIOSDevice();
	static FReply RemoveFromAndroidDevice();
private:
#ifdef ENABLE_LAUNCHER_DELEGATE
	void OnCreateLauncher(ILauncherRef LauncherRef);
	void OnLaunch(ILauncherWorkerPtr LauncherWorkerPtr, ILauncherProfileRef LauncherProfileRef);
	void OnLauncherCanceled(double ExecutionTime);
	void OnLauncherFinished(bool Outcome, double ExecutionTime, int32 ReturnCode);
	void RemoveFromDevice();
#endif
	static bool TryConstructMobileCommandLineArgumentsFile(FString& CommandLineArgsFile);
	static bool TryPushCommandLineArgsToDevice(const FString& Executable, const FString& ExeArguments, const FString& CommandLineArgsFile);
private:
#ifdef ENABLE_LAUNCHER_DELEGATE
	bool bAndroidDevice;
#endif
};
