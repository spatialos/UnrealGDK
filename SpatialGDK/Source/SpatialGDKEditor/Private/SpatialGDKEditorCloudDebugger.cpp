// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorCloudDebugger.h"

#include "ISessionFrontendModule.h"
#include "Modules/ModuleManager.h"
#include "ITcpMessagingModule.h"
#include "SpatialGDKServicesModule.h"
#include "Logging/LogMacros.h"
#include "SpatialGDKEditor.h"

FSpatialGDKEditorCloudDebugger::FSpatialGDKEditorCloudDebugger()
{
	FModuleManager::LoadModuleChecked<ISessionFrontendModule>("SessionFrontend").DebugWorkerDelegate.BindRaw(this, &FSpatialGDKEditorCloudDebugger::DebugWorker);
}

void FSpatialGDKEditorCloudDebugger::DebugWorker(const FString& InDeploymentName, const FString& InWorkerId)
{
	FModuleManager::LoadModuleChecked<ITcpMessagingModule>("TcpMessaging").AddOutgoingConnection("127.0.0.1:8666");

	FString SpatialArgs = FString::Printf(TEXT("project deployment worker port-forward -d=%s -w=%s -p=8666"), *InDeploymentName, *InWorkerId);

	FProcHandle handle = FPlatformProcess::CreateProc(*FSpatialGDKServicesModule::GetSpatialExe(), *SpatialArgs, true, false, false, NULL, 0, *FSpatialGDKServicesModule::GetSpatialOSDirectory(), NULL);
	if (handle.IsValid())
	{
		UE_LOG(LogSpatialGDKEditor, Warning, TEXT("handle is valid"));
	}
	else
	{
		UE_LOG(LogSpatialGDKEditor, Error, TEXT("handle is invalid"));
	}
}
