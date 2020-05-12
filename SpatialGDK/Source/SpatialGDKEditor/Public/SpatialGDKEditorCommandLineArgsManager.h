// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "Templates/SharedPointer.h"

class FReply;

typedef TSharedRef<class ILauncher> ILauncherRef;
typedef TSharedPtr<class ILauncherWorker> ILauncherWorkerPtr;
typedef TSharedRef<class ILauncherProfile> ILauncherProfileRef;

//#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 24
//#define ENABLE_LAUNCHER_DELEGATE
//#endif

class FSpatialGDKEditorCommandLineArgsManager
{
public:
	FSpatialGDKEditorCommandLineArgsManager();

	void Startup();

	FReply GenerateDevAuthToken();
	FReply PushToIOSDevice();
	FReply PushToAndroidDevice();
	FReply RemoveFromIOSDevice();
	FReply RemoveFromAndroidDevice();
private:
#ifdef ENABLE_LAUNCHER_DELEGATE
	void OnCreateLauncher(ILauncherRef LauncherRef);
	void OnLaunch(ILauncherWorkerPtr LauncherWorkerPtr, ILauncherProfileRef LauncherProfileRef);
	void OnLauncherCanceled(double ExecutionTime);
	void OnLauncherFinished(bool Outcome, double ExecutionTime, int32 ReturnCode);
	void RemoveFromDevice();
#endif
	bool TryConstructMobileCommandLineArgumentsFile(FString& CommandLineArgsFile);
	bool TryPushCommandLineArgsToDevice(const FString& Executable, const FString& ExeArguments, const FString& CommandLineArgsFile);
private:
#ifdef ENABLE_LAUNCHER_DELEGATE
	bool bAndroidDevice;
	bool bIOSDevice;
#endif
};
