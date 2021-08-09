#pragma once

#include "DefaultServerWorkerStartupHandler.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/Startup/ServerWorkerStartupCommon.h"
#include "Interop/Startup/SpatialStartupCommon.h"

class USpatialNetDriver;

namespace SpatialGDK
{
class ISpatialOSWorker;

class FCreateServerWorkerStep : public FStartupStep
{
public:
	FCreateServerWorkerStep(TSharedRef<FServerWorkerStartupContext> InState, USpatialNetDriver& InNetDriver,
							ISpatialOSWorker& InConnection);

	virtual void Start() override;

	virtual bool TryFinish() override;

private:
	TSharedRef<FServerWorkerStartupContext> State;

	USpatialNetDriver* NetDriver;
	ISpatialOSWorker* Connection;

	TOptional<ServerWorkerEntityCreator> WorkerEntityCreator;
};

struct FWaitForServerWorkersStep final : public FStartupStep
{
	FWaitForServerWorkersStep(TSharedRef<FServerWorkerStartupContext> InState, FInitialSetup InSetup, ISpatialOSWorker& InConnection);

	virtual bool TryFinish() override;

	TSharedRef<FServerWorkerStartupContext> State;
	FInitialSetup Setup;
	ISpatialOSWorker* Connection;
};

class FWaitForGsmEntityStep : public FStartupStep
{
public:
	FWaitForGsmEntityStep(ISpatialOSWorker& InConnection);

	virtual bool TryFinish() override;

private:
	ISpatialOSWorker* Connection;
};

class FDeriveDeploymentStartupStateStep : public FStartupStep
{
public:
	FDeriveDeploymentStartupStateStep(TSharedRef<FServerWorkerStartupContext> InState, ISpatialOSWorker& InConnection,
									  UGlobalStateManager& InGlobalStateManager);

	virtual bool TryFinish() override;

private:
	TSharedRef<FServerWorkerStartupContext> State;
	ISpatialOSWorker* Connection;
	UGlobalStateManager* GlobalStateManager;
};
} // namespace SpatialGDK
