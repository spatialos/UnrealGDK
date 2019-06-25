// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "EditorWorkerController.h"

#include "Private/SpatialGDKServicesPrivate.h"

#if WITH_EDITOR
namespace
{
struct EditorWorkerController
{
	void OnPrePIEEnded(bool bValue)
	{
		LastPIEEndTime = FDateTime::Now().ToUnixTimestamp();
		FEditorDelegates::PrePIEEnded.Remove(PIEEndHandle);
	}

	void OnSpatialShutdown()
	{
		LastPIEEndTime = 0;	// Reset PIE end time to ensure replace-a-worker isn't called
		FEditorDelegates::PrePIEEnded.Remove(PIEEndHandle);
	}

	void InitWorkers(const FString& WorkerType)
	{
		ReplaceProcesses.Empty();

		// Only issue the worker replace request if there's a chance the load balancer hasn't acknowledged
		// that the previous session's workers have disconnected. There's no hard `heartbeat` time for this as
		// it's dependent on multiple factors (fabric load etc.), so this value was landed on after significant
		// trial and error.
		const int64 WorkerReplaceThresholdSeconds = 8;

		int64 SecondsSinceLastSession = FDateTime::Now().ToUnixTimestamp() - LastPIEEndTime;
		UE_LOG(LogSpatialGDKServices, Verbose, TEXT("Seconds since last session - %d"), SecondsSinceLastSession);

		PIEEndHandle = FEditorDelegates::PrePIEEnded.AddRaw(this, &EditorWorkerController::OnPrePIEEnded);

		int32 PlayNumberOfServers;
		GetDefault<ULevelEditorPlaySettings>()->GetPlayNumberOfServers(PlayNumberOfServers);

		WorkerIds.SetNum(PlayNumberOfServers);
		for (int i = 0; i < PlayNumberOfServers; ++i)
		{
			FString NewWorkerId = WorkerType + FGuid::NewGuid().ToString();

			if (!WorkerIds[i].IsEmpty() && SecondsSinceLastSession < WorkerReplaceThresholdSeconds)
			{
				ReplaceProcesses.Add(ReplaceWorker(WorkerIds[i], NewWorkerId));
			}

			WorkerIds[i] = NewWorkerId;
		}
	}

	FProcHandle ReplaceWorker(const FString& OldWorker, const FString& NewWorker)
	{
		const FString CmdExecutable = TEXT("spatial.exe");

		const FString CmdArgs = FString::Printf(
			TEXT("local worker replace "
				"--existing_worker_id %s "
				"--replacing_worker_id %s"), *OldWorker, *NewWorker);
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
				FPlatformProcess::Sleep(0.1f);
			}
		}
	}

	TArray<FString> WorkerIds;
	TArray<FProcHandle> ReplaceProcesses;
	int64 LastPIEEndTime = 0;	// Unix epoch time in seconds
	FDelegateHandle PIEEndHandle;
	FDelegateHandle SpatialShutdownHandle;
};

static EditorWorkerController WorkerController;
}

namespace SpatialGDKServices
{
void InitWorkers(const FString& WorkerType, bool bConnectAsClient, FString& OutWorkerId)
{
	const bool bSingleThreadedServer = !bConnectAsClient && (GPlayInEditorID > 0);
	const int32 FirstServerEditorID = 1;
	if (bSingleThreadedServer)
	{
		if (GPlayInEditorID == FirstServerEditorID)
		{
			WorkerController.InitWorkers(WorkerType);
		}

		WorkerController.BlockUntilWorkerReady(GPlayInEditorID - 1);
		OutWorkerId = WorkerController.WorkerIds[GPlayInEditorID - 1];
	}
}

void OnSpatialShutdown()
{
	WorkerController.OnSpatialShutdown();
}

} // namespace SpatialGDKServices
#endif // WITH_EDITOR
