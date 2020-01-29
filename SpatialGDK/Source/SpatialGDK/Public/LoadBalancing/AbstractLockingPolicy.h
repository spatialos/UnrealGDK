// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/AbstractSpatialPackageMapClient.h"
#include "EngineClasses/AbstractVirtualWorkerTranslator.h"
#include "Interop/SpatialStaticComponentView.h"
#include "SpatialConstants.h"

#include "GameFramework/Actor.h"
#include "Improbable/SpatialEngineDelegates.h"
#include "Templates/SharedPointer.h"
#include "UObject/WeakObjectPtrTemplates.h"

#include "AbstractLockingPolicy.generated.h"

UCLASS(abstract)
class SPATIALGDK_API UAbstractLockingPolicy : public UObject
{
	GENERATED_BODY()

public:
	virtual void Init(USpatialStaticComponentView* InStaticComponentView, UAbstractSpatialPackageMapClient* InPackageMap,
		AbstractVirtualWorkerTranslator* InVirtualWorkerTranslator, SpatialDelegates::FAcquireLockDelegate& AcquireLockDelegate,
		SpatialDelegates::FReleaseLockDelegate& ReleaseLockDelegate)
	{
		StaticComponentView = InStaticComponentView;
		PackageMap = InPackageMap;
		VirtualWorkerTranslator = InVirtualWorkerTranslator;

		AcquireLockDelegate.BindUObject(this, &UAbstractLockingPolicy::AcquireLockFromDelegate);
		ReleaseLockDelegate.BindUObject(this, &UAbstractLockingPolicy::ReleaseLockFromDelegate);
	};
	virtual ActorLockToken AcquireLock(AActor* Actor, FString LockName = TEXT("")) PURE_VIRTUAL(UAbstractLockingPolicy::AcquireLock, return SpatialConstants::INVALID_ACTOR_LOCK_TOKEN;);
	virtual void ReleaseLock(ActorLockToken Token) PURE_VIRTUAL(UAbstractLockingPolicy::ReleaseLock, return;);
	virtual bool IsLocked(const AActor* Actor) const PURE_VIRTUAL(UAbstractLockingPolicy::IsLocked, return false;);

protected:
	TWeakObjectPtr<USpatialStaticComponentView> StaticComponentView;
	TWeakObjectPtr<UAbstractSpatialPackageMapClient> PackageMap;
	AbstractVirtualWorkerTranslator* VirtualWorkerTranslator;

private:
	virtual bool AcquireLockFromDelegate(AActor* ActorToLock, const FString& DelegateLockIdentifier) PURE_VIRTUAL(UAbstractLockingPolicy::AcquireLockFromDelegate, return false;);
	virtual void ReleaseLockFromDelegate(AActor* ActorToRelease, const FString& DelegateLockIdentifier) PURE_VIRTUAL(UAbstractLockingPolicy::ReleaseLockFromDelegate, return;);
};
