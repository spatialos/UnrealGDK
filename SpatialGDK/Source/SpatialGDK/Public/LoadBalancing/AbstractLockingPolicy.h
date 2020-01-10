#pragma once

#include "SpatialConstants.h"
#include "EngineClasses/AbstractPackageMap.h"
#include "Interop/AbstractStaticComponentView.h"

#include "GameFramework/Actor.h"

#include <memory>

#include "AbstractLockingPolicy.generated.h"

UCLASS(abstract)
class SPATIALGDK_API UAbstractLockingPolicy : public UObject
{
	GENERATED_BODY()

public:
	virtual void Init(AbstractStaticComponentView* InStaticComponentView, AbstractPackageMap* InPackageMap) {
		StaticComponentView = std::make_shared<AbstractStaticComponentView>(*InStaticComponentView);
		PackageMap = std::make_shared<AbstractPackageMap>(*InPackageMap);
	};
	virtual ActorLockToken AcquireLock(AActor* Actor, FString LockName = "") PURE_VIRTUAL(UAbstractLockingPolicy::AcquireLock, return SpatialConstants::INVALID_ACTOR_LOCK_TOKEN;);
	virtual void ReleaseLock(ActorLockToken Token) PURE_VIRTUAL(UAbstractLockingPolicy::ReleaseLock, return;);
	virtual bool IsLocked(const AActor* Actor) const PURE_VIRTUAL(UAbstractLockingPolicy::IsLocked, return false;);

protected:
	std::weak_ptr<AbstractStaticComponentView> StaticComponentView;
	std::weak_ptr<AbstractPackageMap> PackageMap;
};
