// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorCloudDebugger.h"


#include "Modules/ModuleManager.h"
#include "SpatialGDKServicesModule.h"
#include "Logging/LogMacros.h"
#include "SpatialGDKEditor.h"

FSpatialGDKEditorCloudDebugger::FSpatialGDKEditorCloudDebugger()
{
	SessionFrontendModule = &FModuleManager::LoadModuleChecked<ISessionFrontendModule>("SessionFrontend");
	TcpMessagingModule = &FModuleManager::LoadModuleChecked<ITcpMessagingModule>("TcpMessaging");

	SessionFrontendModule->DebugWorkerDelegate.BindRaw(this, &FSpatialGDKEditorCloudDebugger::DebugWorker);
	TcpMessagingModule->AddOutgoingConnection("127.0.0.1:6667");
}

FSpatialGDKEditorCloudDebugger::~FSpatialGDKEditorCloudDebugger()
{
	ClosePortForward();
	TcpMessagingModule->RemoveOutgoingConnection("127.0.0.1:6667");
}

void FSpatialGDKEditorCloudDebugger::DebugWorker(const FString& InDeploymentName, const FString& InWorkerId)
{
	ClosePortForward();

	ForceSpatialLogin(); // to avoid polluted output if it is the first spatial command used

	FString SpatialExe = FSpatialGDKServicesModule::GetSpatialExe();
	FString SpatialArgs = FString::Printf(TEXT("project deployment worker port-forward -d=%s -w=%s -p=6667"), *InDeploymentName, *InWorkerId);

	void* PipeRead = nullptr;
	void* PipeWrite = nullptr;
	verify(FPlatformProcess::CreatePipe(PipeRead, PipeWrite));

	PortForwardHandle = FPlatformProcess::CreateProc(*SpatialExe, *SpatialArgs, true, false, false, NULL, 0, *FSpatialGDKServicesModule::GetSpatialOSDirectory(), PipeWrite);
	if (PortForwardHandle.IsValid())
	{
		FString fullMsg = TEXT("");
		FString currentMsg = TEXT("");
		do 
		{
			FPlatformProcess::Sleep(0.1);
			currentMsg = FPlatformProcess::ReadPipe(PipeRead);
			fullMsg += currentMsg;
		} while ((!currentMsg.IsEmpty() || fullMsg.IsEmpty())
			&& PortForwardHandle.IsValid());

		
		if (fullMsg.Find(TEXT("level=error")) > 0)
		{
			UE_LOG(LogSpatialGDKEditor, Error, TEXT("Tcp port forwarding process returned error: %s"), *fullMsg);
			ClosePortForward();
		}
		else if(fullMsg.Find(TEXT("level=info")) > 0)
		{
			UE_LOG(LogSpatialGDKEditor, Log, TEXT("Tcp port forwarding process started successfully"));
		}
		else
		{
			UE_LOG(LogSpatialGDKEditor, Error, TEXT("Tcp port forwarding process returned unexpected message: %s", *fullMsg));
			ClosePortForward();
		}
	}
	else
	{
		UE_LOG(LogSpatialGDKEditor, Error, TEXT("Creating tcp port forwarding process failed!"));
	}

	FPlatformProcess::ClosePipe(PipeRead, PipeWrite);
}

void FSpatialGDKEditorCloudDebugger::ForceSpatialLogin()
{
	FString SpatialArgs = TEXT("auth login");
	FPlatformProcess::ExecProcess(*FSpatialGDKServicesModule::GetSpatialExe(), *SpatialArgs, NULL, NULL, NULL, *FSpatialGDKServicesModule::GetSpatialOSDirectory());
}

void FSpatialGDKEditorCloudDebugger::ClosePortForward()
{
	if (PortForwardHandle.IsValid())
	{
		FPlatformProcess::TerminateProc(PortForwardHandle, true);
		FPlatformProcess::CloseProc(PortForwardHandle);
		UE_LOG(LogSpatialGDKEditor, Log, TEXT("Tcp port forwarding process closed"));
	}
}
