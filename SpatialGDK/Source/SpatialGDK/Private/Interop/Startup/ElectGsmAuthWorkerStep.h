#pragma once

#include "Containers/Array.h"
#include "Misc/Optional.h"
#include "Templates/SharedPointer.h"
#include "Templates/UniquePtr.h"

#include "Interop/Startup/SpatialStartupCommon.h"

class UGlobalStateManager;

namespace SpatialGDK
{
struct FServerWorkerStartupContext;

class FElectGsmAuthWorkerStep : public FStartupStep
{
public:
	FElectGsmAuthWorkerStep(TSharedRef<FServerWorkerStartupContext> InState, UGlobalStateManager& InGlobalStateManager);

	virtual void Start() override;
	virtual bool TryFinish() override;
	virtual FString Describe() const override;

private:
	bool TryClaimingPartition();

	TSharedRef<FServerWorkerStartupContext> State;
	UGlobalStateManager* GlobalStateManager;

	TOptional<bool> bShouldHaveGsmAuthority;
};
} // namespace SpatialGDK
