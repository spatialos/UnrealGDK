// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#pragma once

#include <improbable/worker.h>
#include <improbable/view.h>
#include <unreal/generated/UnrealCharacter.h>
#include <unreal/core_types.h>
#include "SpatialHandlePropertyMap.h"
#include "SpatialTypeBinding.h"
#include "SpatialTypeBinding_Character.generated.h"

UCLASS()
class USpatialTypeBinding_Character : public USpatialTypeBinding
{
	GENERATED_BODY()
public:
	static const FRepHandlePropertyMap& GetHandlePropertyMap();
	void BindToView() override;
	void UnbindFromView() override;
	worker::ComponentId GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const override;
	worker::Entity CreateActorEntity(const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges) const override;
	void SendComponentUpdates(const FPropertyChangeState& Changes, const worker::EntityId& EntityId) const override;
private:
	worker::Dispatcher::CallbackKey SingleClientAddCallback;
	worker::Dispatcher::CallbackKey SingleClientUpdateCallback;
	worker::Dispatcher::CallbackKey MultiClientAddCallback;
	worker::Dispatcher::CallbackKey MultiClientUpdateCallback;

	// Helper functions.
	void BuildSpatialComponentUpdate(
		const FPropertyChangeState& Changes,
		improbable::unreal::UnrealCharacterSingleClientReplicatedData::Update& SingleClientUpdate,
		bool& bSingleClientUpdateChanged,
		improbable::unreal::UnrealCharacterMultiClientReplicatedData::Update& MultiClientUpdate,
		bool& bMultiClientUpdateChanged) const;
	void ApplyUpdateToSpatial_SingleClient(
		const uint8* RESTRICT Data,
		int32 Handle,
		UProperty* Property,
		improbable::unreal::UnrealCharacterSingleClientReplicatedData::Update& OutUpdate) const;
	void ApplyUpdateToSpatial_MultiClient(
		const uint8* RESTRICT Data,
		int32 Handle,
		UProperty* Property,
		improbable::unreal::UnrealCharacterMultiClientReplicatedData::Update& OutUpdate) const;
	void ReceiveUpdateFromSpatial_SingleClient(
		worker::EntityId EntityId,
		const improbable::unreal::UnrealCharacterSingleClientReplicatedData::Update& Update) const;
	void ReceiveUpdateFromSpatial_MultiClient(
		worker::EntityId EntityId,
		const improbable::unreal::UnrealCharacterMultiClientReplicatedData::Update& Update) const;
};
