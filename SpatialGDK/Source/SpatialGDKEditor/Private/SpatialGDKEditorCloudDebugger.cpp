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

	FString SpatialExe = FSpatialGDKServicesModule::GetSpatialExe();
	FString SpatialArgs = FString::Printf(TEXT("project deployment worker port-forward -d=%s -w=%s -p=6667"), *InDeploymentName, *InWorkerId);

	PortForwardHandle = FPlatformProcess::CreateProc(*SpatialExe, *SpatialArgs, true, false, false, NULL, 0, *FSpatialGDKServicesModule::GetSpatialOSDirectory(), NULL);
	if (!PortForwardHandle.IsValid())
	{
		UE_LOG(LogSpatialGDKEditor, Error, TEXT("Creating tcp port forwarding failed!"));
	}
}

void FSpatialGDKEditorCloudDebugger::ClosePortForward()
{
	if (PortForwardHandle.IsValid())
	{
		FPlatformProcess::TerminateProc(PortForwardHandle, true);
		FPlatformProcess::CloseProc(PortForwardHandle);
	}
}
