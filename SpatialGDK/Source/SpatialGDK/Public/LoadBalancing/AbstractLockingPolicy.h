#pragma once

#include "SpatialConstants.h"
#include "EngineClasses/AbstractPackageMapClient.h"
#include "EngineClasses/AbstractVirtualWorkerTranslator.h"
#include "Interop/AbstractStaticComponentView.h"

#include "GameFramework/Actor.h"

#include <memory>

#include "AbstractLockingPolicy.generated.h"

UCLASS(abstract)
class SPATIALGDK_API UAbstractLockingPolicy : public UObject
{
	GENERATED_BODY()

public:
	virtual void Init(UAbstractStaticComponentView* InStaticComponentView, UAbstractPackageMapClient* InPackageMap, TSharedPtr<AbstractVirtualWorkerTranslator> InVirtualWorkerTranslator) {
		StaticComponentView = InStaticComponentView;
		PackageMap = InPackageMap;
		VirtualWorkerTranslator = InVirtualWorkerTranslator;
	};
	virtual ActorLockToken AcquireLock(AActor* Actor, FString LockName = "") PURE_VIRTUAL(UAbstractLockingPolicy::AcquireLock, return SpatialConstants::INVALID_ACTOR_LOCK_TOKEN;);
	virtual void ReleaseLock(ActorLockToken Token) PURE_VIRTUAL(UAbstractLockingPolicy::ReleaseLock, return;);
	virtual bool IsLocked(const AActor* Actor) const PURE_VIRTUAL(UAbstractLockingPolicy::IsLocked, return false;);

protected:
	TWeakObjectPtr<UAbstractStaticComponentView> StaticComponentView;
	TWeakObjectPtr<UAbstractPackageMapClient> PackageMap;
	TWeakPtr<AbstractVirtualWorkerTranslator> VirtualWorkerTranslator;
};
