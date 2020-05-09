// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "Templates/SharedPointer.h"

class FReply;

typedef TSharedRef<class ILauncher> ILauncherRef;
typedef TSharedPtr<class ILauncherWorker> ILauncherWorkerPtr;
typedef TSharedRef<class ILauncherProfile> ILauncherProfileRef;

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
	void OnCreateLauncher(ILauncherRef LauncherRef);
	void OnLaunch(ILauncherWorkerPtr LauncherWorkerPtr, ILauncherProfileRef LauncherProfileRef);
	void OnLauncherCanceled(double ExecutionTime);

	bool TryConstructMobileCommandLineArgumentsFile(FString& CommandLineArgsFile);
	bool TryPushCommandLineArgsToDevice(const FString& Executable, const FString& ExeArguments, const FString& CommandLineArgsFile);
private:
	bool bAndroidDevice;
	bool bIOSDevice;
};
