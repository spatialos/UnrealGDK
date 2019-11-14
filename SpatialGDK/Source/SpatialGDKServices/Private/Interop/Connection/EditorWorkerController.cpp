// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/EditorWorkerController.h"

#include "SpatialGDKServicesPrivate.h"

#include "Editor.h"

#if WITH_EDITOR
namespace
{
struct EditorWorkerController
{
	// Only issue the worker replace request if there's a chance the load balancer hasn't acknowledged
	// that the previous session's workers have disconnected. There's no hard `heartbeat` time for this as
	// it's dependent on multiple factors (fabric load etc.), so this value was landed on after significant
	// trial and error.
	const int64 WorkerReplaceThresholdSeconds = 8;

	const FString ServicePort = TEXT("9876");

	void OnPrePIEEnded(bool bValue)
	{
		LastPIEEndTime = FDateTime::Now().ToUnixTimestamp();
		FEditorDelegates::PrePIEEnded.Remove(PIEEndHandle);
		bHasInitialized = false;
	}

	void OnSpatialShutdown()
	{
		LastPIEEndTime = 0;	// Reset PIE end time to ensure replace-a-worker isn't called
		FEditorDelegates::PrePIEEnded.Remove(PIEEndHandle);
	}

	void InitWorkers()
	{
		int64 SecondsSinceLastSession = FDateTime::Now().ToUnixTimestamp() - LastPIEEndTime;
		UE_LOG(LogSpatialGDKServices, Verbose, TEXT("Seconds since last session - %d"), SecondsSinceLastSession);

		PIEEndHandle = FEditorDelegates::PrePIEEnded.AddRaw(this, &EditorWorkerController::OnPrePIEEnded);

		const ULevelEditorPlaySettings* LevelEditorPlaySettings = GetDefault<ULevelEditorPlaySettings>();
		const int32 WorkerCount = LevelEditorPlaySettings->GetTotalPIEServerWorkerCount();
		WorkerIds.SetNum(WorkerCount);
		ReplaceProcesses.SetNum(WorkerCount);

		int32 WorkerIdIndex = 0;
		for (const TPair<FName, int32>& WorkerType : LevelEditorPlaySettings->GetPIEServerWorkers())
		{
			for (int i = 0; i < WorkerType.Value; ++i)
			{
				const FString NewWorkerId = WorkerType.Key.ToString() + FGuid::NewGuid().ToString();

				if (!WorkerIds[WorkerIdIndex].IsEmpty() && SecondsSinceLastSession < WorkerReplaceThresholdSeconds)
				{
					ReplaceProcesses.Add(ReplaceWorker(WorkerIds[WorkerIdIndex], NewWorkerId));
				}

				WorkerIds[WorkerIdIndex] = NewWorkerId;
				WorkerIdIndex++;
			}
		}

		bHasInitialized = true;
	}

	FProcHandle ReplaceWorker(const FString& OldWorker, const FString& NewWorker)
	{
		const FString CmdExecutable = TEXT("spatial.exe");

		const FString CmdArgs = FString::Printf(
			TEXT("local worker replace "
				"--local_service_grpc_port %s "
				"--existing_worker_id %s "
				"--replacing_worker_id %s"), *ServicePort, *OldWorker, *NewWorker);
		uint32 ProcessID = 0;
		FProcHandle ProcHandle = FPlatformProcess::CreateProc(
			*(CmdExecutable), *CmdArgs, false, true, true, &ProcessID, 2 /*PriorityModifier*/,
			nullptr, nullptr, nullptr);

		return ProcHandle;
	}

	void BlockUntilWorkerReady(int32 WorkerIdx)
	{
		if (WorkerIdx < ReplaceProcesses.Num())
		{
			while (FPlatformProcess::IsProcRunning(ReplaceProcesses[WorkerIdx]))
			{
				// Only block until the worker connection will have timed out.
				if ((FDateTime::Now().ToUnixTimestamp() - LastPIEEndTime) < WorkerReplaceThresholdSeconds)
				{
					FPlatformProcess::Sleep(0.1f);
				}
			}
		}
	}

	TArray<FString> WorkerIds;
	TArray<FProcHandle> ReplaceProcesses;
	int64 LastPIEEndTime = 0;	// Unix epoch time in seconds
	FDelegateHandle PIEEndHandle;
	FDelegateHandle SpatialShutdownHandle;
	bool bHasInitialized = false;
};

static EditorWorkerController WorkerController;
} // end namespace

namespace SpatialGDKServices
{
void InitWorkers(bool bConnectAsClient, int32 PlayInEditorID, FString& OutWorkerId)
{
	const bool bSingleThreadedServer = !bConnectAsClient && (PlayInEditorID > 0);
	const int32 FirstServerEditorID = 1;
	if (bSingleThreadedServer)
	{
		if (!WorkerController.bHasInitialized)
		{
			WorkerController.InitWorkers();
		}

		WorkerController.BlockUntilWorkerReady(PlayInEditorID - 1);
		OutWorkerId = WorkerController.WorkerIds[PlayInEditorID - 1];
	}
}

void OnSpatialShutdown()
{
	WorkerController.OnSpatialShutdown();
}

} // namespace SpatialGDKServices
#endif // WITH_EDITOR
