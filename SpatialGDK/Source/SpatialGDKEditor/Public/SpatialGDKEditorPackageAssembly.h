// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKEditorPackageAssembly, Log, All);


class SPATIALGDKEDITOR_API FSpatialGDKPackageAssembly : public TSharedFromThis<FSpatialGDKPackageAssembly>
{
public:
	FSpatialGDKPackageAssembly();

	void BuildServerWorker();
	void BuildClientWorker();
	void BuildSimulatedPlayerWorker();
	void BuildAll();
	bool CanBuild() const;

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
	} CurrentAssemblyTarget;

	void BuildNext();
};
