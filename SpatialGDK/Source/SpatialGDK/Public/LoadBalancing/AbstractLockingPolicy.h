// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/AbstractPackageMapClient.h"
#include "EngineClasses/AbstractVirtualWorkerTranslator.h"
#include "Interop/SpatialStaticComponentView.h"
#include "SpatialConstants.h"

#include "GameFramework/Actor.h"

#include "AbstractLockingPolicy.generated.h"

UCLASS(abstract)
class SPATIALGDK_API UAbstractLockingPolicy : public UObject
{
	GENERATED_BODY()

public:
	virtual void Init(USpatialStaticComponentView* InStaticComponentView, UAbstractPackageMapClient* InPackageMap, TSharedPtr<AbstractVirtualWorkerTranslator> InVirtualWorkerTranslator) {
		StaticComponentView = InStaticComponentView;
		PackageMap = InPackageMap;
		VirtualWorkerTranslator = InVirtualWorkerTranslator;
	};
	virtual ActorLockToken AcquireLock(AActor* Actor, FString LockName = "") PURE_VIRTUAL(UAbstractLockingPolicy::AcquireLock, return -1;);
	virtual void ReleaseLock(ActorLockToken Token) PURE_VIRTUAL(UAbstractLockingPolicy::ReleaseLock, return;);
	virtual bool IsLocked(const AActor* Actor) const PURE_VIRTUAL(UAbstractLockingPolicy::IsLocked, return false;);

protected:
	TWeakObjectPtr<USpatialStaticComponentView> StaticComponentView;
	TWeakObjectPtr<UAbstractPackageMapClient> PackageMap;
	TWeakPtr<AbstractVirtualWorkerTranslator> VirtualWorkerTranslator;
};
