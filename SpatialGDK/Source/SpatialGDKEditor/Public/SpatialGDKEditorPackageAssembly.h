// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Logging/LogMacros.h"
#include "Misc/MonitoredProcess.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKEditorPackageAssembly, Log, All);

enum class EPackageAssemblyStatus
{
	CANCELED = -1,
	STARTED = 0,
	COMPLETED = 1,
	FAILED = 2,
};

DECLARE_DELEGATE_TwoParams(FSpatialGDKPackageAssemblyStatus, FString, EPackageAssemblyStatus);

class SPATIALGDKEDITOR_API FSpatialGDKPackageAssembly : public TSharedFromThis<FSpatialGDKPackageAssembly>
{
public:
	FSpatialGDKPackageAssembly();

	void BuildServerWorker();
	void BuildClientWorker();
	void BuildSimulatedPlayerWorker();
	void BuildAll();
	bool CanBuild() const;

	void UploadAssembly(const FString &name, bool Force);

	FSpatialGDKPackageAssemblyStatus OnPackageAssemblyStatus;

private:
	enum class EPackageAssemblyTarget
	{
		NONE = 0,
		START_ALL,
		BUILD_CLIENT,
		ZIP_CLIENT,
		BUILD_SERVER,
		ZIP_SERVER,
		BUILD_SIMULATED_PLAYERS,
		ZIP_SIMULATED_PLAYERS,
		UPLOAD_ASSEMBLY,
	} CurrentAssemblyTarget;

	TUniquePtr<FMonitoredProcess> Upload;

	void BuildNext();
	void OnUploadCompleted(int32);
	void OnUploadOutput(FString);
	void OnUploadCanceled();
};
