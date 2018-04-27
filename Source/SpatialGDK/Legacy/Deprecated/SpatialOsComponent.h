// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#include <improbable/standard_library.h>

#include "ComponentId.h"
#include "Components/ActorComponent.h"
#include "EntityId.h"
#include "ScopedViewCallbacks.h"
#include "SpatialOSComponent.generated.h"

class UCommander;
class UCallbackDispatcher;
class UAddComponentOpWrapperBase;
class UEntityPipeline;

UENUM(BlueprintType)
enum class EAuthority : uint8
{
	NotAuthoritative = 0 UMETA(DisplayName = "Not Authoritative"),
	Authoritative = 1 UMETA(DisplayName = "Authoritative"),
	AuthorityLossImminent = 2 UMETA(DisplayName = "Authority Loss Imminent"),
};

// clang-format off
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSpatialComponentUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpatialAuthorityChangeDelegate, EAuthority, newAuthority);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSpatialComponentReadyDelegate);
// clang-format on

UCLASS(abstract)
class SPATIALGDK_API USpatialOsComponent : public UActorComponent
{
	GENERATED_BODY()

  public:
	USpatialOsComponent();
	virtual ~USpatialOsComponent() = default;

	virtual void BeginDestroy() override;

	virtual void Init(const TWeakPtr<SpatialOSConnection>& InConnection, const TWeakPtr<SpatialOSView>& InView,
					  worker::EntityId InEntityId, UCallbackDispatcher* InCallbackDispatcher);

	virtual void Disable(const worker::EntityId InEntityId, UCallbackDispatcher* CallbackDispatcher)
		PURE_VIRTUAL(USpatialOsComponent::Disable(), );

	UFUNCTION(BlueprintPure, Category = "SpatialOS Component")
	virtual FComponentId GetComponentId() PURE_VIRTUAL(USpatialOsComponent::GetComponentId, return 0;);

	virtual void ApplyInitialState(const UAddComponentOpWrapperBase& AddComponentOp)
		PURE_VIRTUAL(USpatialOsComponent::ApplyInitialState(), );

	virtual void ReplicateChanges(float DeltaSeconds) PURE_VIRTUAL(USpatialOsComponent::ReplicateChanges, );

	virtual void TriggerAutomaticComponentUpdate(float DeltaSeconds)
		PURE_VIRTUAL(USpatialOsComponent::TriggerAutomaticComponentUpdate, );

	void ApplyInitialAuthority(const worker::AuthorityChangeOp& AuthChangeOp);

	worker::EntityId GetEntityId();

	UFUNCTION(BlueprintPure, Category = "SpatialOS Component",
			  meta = (DeprecatedFunction, DeprecationMessage = "Please use GetAuthority() instead."))
	bool HasAuthority();

	UFUNCTION(BlueprintPure, Category = "SpatialOS Component")
	EAuthority GetAuthority();

	UFUNCTION(BlueprintPure, Category = "SpatialOS Component")
	bool IsComponentReady();

	UFUNCTION(BlueprintCallable, Category = "SpatialOS Component")
	void SendAuthorityLossImminentAcknowledgement();

	UPROPERTY(BlueprintAssignable, Category = "SpatialOS Component")
	FSpatialAuthorityChangeDelegate OnAuthorityChange;

	UPROPERTY(BlueprintAssignable, Category = "SpatialOS Component")
	FSpatialComponentReadyDelegate OnComponentReady;

	UFUNCTION(BlueprintPure, Category = "SpatialOS Component")
	UCommander* SendCommand();

	/** Maximum frequency of automatically generated SpatialOS updates from this
  * component.
  * 0 means manual mode. No automatic updates will be sent.
  * Use the manual mode if you want more control over how updates are generated,
  * and how often.
  * Note that the final update rate is dependent on your engine tick frequency.
  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxUpdatesPerSecond;

  protected:
	virtual void OnAuthorityChangeDispatcherCallback(const worker::AuthorityChangeOp& op);
	void OnRemoveComponentDispatcherCallback(const worker::RemoveComponentOp& op);

	TWeakPtr<SpatialOSConnection> Connection;
	TWeakPtr<SpatialOSView> View;
	worker::EntityId EntityId;
	TUniquePtr<improbable::unreal::callbacks::FScopedViewCallbacks> Callbacks;

	float TimeUntilNextUpdate;

	EAuthority Authority;
	bool bIsComponentReady;

	bool bHasEventQueued;

	UPROPERTY()
	UCommander* Commander;
};