// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ScopedViewCallbacks.h"
#include "TestSchema.h"
#include "EntityId.h"
#include "RequestId.h"
#include "ComponentId.h"
#include "CommanderTypes.h"
#include "EntityTemplate.h"
#include "EntityQueryConstraints.h"
#include "EntityQueryCommandResult.h"
#include "SpatialOSCommandResult.h"
#include "SpatialOsComponent.h"
#include "SpatialOSViewTypes.h"
#include "SpatialOSWorkerTypes.h"

#include "Commander.generated.h"

class USpatialOsComponent;
class UTestType2;
class UTestType1;

DECLARE_DYNAMIC_DELEGATE_TwoParams(FReserveEntityIdResultDelegate, const FSpatialOSCommandResult&, result, FEntityId, reservedEntityId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FReserveEntityIdsResultDelegate, const FSpatialOSCommandResult&, result, const TArray<FEntityId>&, reservedEntityIds);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FCreateEntityResultDelegate, const FSpatialOSCommandResult&, result, FEntityId, createdEntityId);
DECLARE_DYNAMIC_DELEGATE_OneParam(FDeleteEntityResultDelegate, const FSpatialOSCommandResult&, result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FEntityQueryCountResultDelegate, UEntityQueryCountCommandResult*, result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FEntityQuerySnapshotResultDelegate, UEntityQuerySnapshotCommandResult*, result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FRequestTestdata2CommandResultDelegate, const FSpatialOSCommandResult&, result, UTestType2*, response);	
DECLARE_DYNAMIC_DELEGATE_TwoParams(FRequestTestdata1CommandResultDelegate, const FSpatialOSCommandResult&, result, UTestType1*, response);	
DECLARE_DYNAMIC_DELEGATE_TwoParams(FTestCommandUserTypeCommandResultDelegate, const FSpatialOSCommandResult&, result, UTestType1*, response);	
DECLARE_DYNAMIC_DELEGATE_TwoParams(FTestCommandCoordinatesCommandResultDelegate, const FSpatialOSCommandResult&, result, FVector, response);	
DECLARE_DYNAMIC_DELEGATE_TwoParams(FTestCommandVector3dCommandResultDelegate, const FSpatialOSCommandResult&, result, FVector, response);	
DECLARE_DYNAMIC_DELEGATE_TwoParams(FTestCommandVector3fCommandResultDelegate, const FSpatialOSCommandResult&, result, FVector, response);	

/**
*
*/
UCLASS(BlueprintType)
class SPATIALOS_API UCommander : public UObject
{
	GENERATED_BODY()

public:
	UCommander();

	UCommander* Init(USpatialOsComponent* component, const TWeakPtr<SpatialOSConnection>& InConnection, const TWeakPtr<SpatialOSView>& InView);

	virtual void BeginDestroy() override;
		
	UFUNCTION(BlueprintCallable, Category = "SpatialOS Commands")
	FRequestId ReserveEntityId(const FReserveEntityIdResultDelegate& callback, int timeoutMs);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS Commands")
	FRequestId ReserveEntityIds(int NumEntitiesToReserve, const FReserveEntityIdsResultDelegate& callback, int timeoutMs);
	    
	UFUNCTION(BlueprintCallable, Category = "SpatialOS Commands")
	FRequestId CreateEntity(UEntityTemplate* entityTemplate, FEntityId entityId, const FCreateEntityResultDelegate& callback, int timeoutMs);
	    
	UFUNCTION(BlueprintCallable, Category = "SpatialOS Commands")
	FRequestId DeleteEntity(FEntityId entityId, const FDeleteEntityResultDelegate& callback, int timeoutMs);

	UFUNCTION(BlueprintCallable, Category = "Commands") 
	FRequestId EntityQueryCountRequest(UEntityQueryConstraint* EntityQuery, const FEntityQueryCountResultDelegate& callback, int timeoutMs);

	UFUNCTION(BlueprintCallable, Category = "Commands", meta = (AutoCreateRefTerm = "ComponentIds"))
	FRequestId EntityQuerySnapshotRequest(UEntityQueryConstraint* EntityQuery, const TArray<FComponentId>& ComponentIds, const FEntityQuerySnapshotResultDelegate& callback, int timeoutMs);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS Commands")
	FRequestId RequestTestdata2(FEntityId entityId, UTestType1* request, const FRequestTestdata2CommandResultDelegate& callback, int timeoutMs, ECommandDelivery commandDelivery = ECommandDelivery::ROUNDTRIP);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS Commands")
	FRequestId RequestTestdata1(FEntityId entityId, UTestType2* request, const FRequestTestdata1CommandResultDelegate& callback, int timeoutMs, ECommandDelivery commandDelivery = ECommandDelivery::ROUNDTRIP);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS Commands")
	FRequestId TestCommandUserType(FEntityId entityId, UTestType1* request, const FTestCommandUserTypeCommandResultDelegate& callback, int timeoutMs, ECommandDelivery commandDelivery = ECommandDelivery::ROUNDTRIP);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS Commands")
	FRequestId TestCommandCoordinates(FEntityId entityId, FVector request, const FTestCommandCoordinatesCommandResultDelegate& callback, int timeoutMs, ECommandDelivery commandDelivery = ECommandDelivery::ROUNDTRIP);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS Commands")
	FRequestId TestCommandVector3d(FEntityId entityId, FVector request, const FTestCommandVector3dCommandResultDelegate& callback, int timeoutMs, ECommandDelivery commandDelivery = ECommandDelivery::ROUNDTRIP);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS Commands")
	FRequestId TestCommandVector3f(FEntityId entityId, FVector request, const FTestCommandVector3fCommandResultDelegate& callback, int timeoutMs, ECommandDelivery commandDelivery = ECommandDelivery::ROUNDTRIP);

	static ECommandResponseCode GetCommandResponseCode(const worker::StatusCode UnderlyingStatusCode);

private:
	TWeakPtr<SpatialOSConnection> Connection;
	TWeakPtr<SpatialOSView> View;

	UPROPERTY()
	USpatialOsComponent* Component;

	UPROPERTY()
	UEntityQueryCountCommandResult* EntityQueryCountCommandResult;

	UPROPERTY()
	UEntityQuerySnapshotCommandResult* EntityQuerySnapshotCommandResult;

	UPROPERTY()
	UTestType2* RequestTestdata2Response;
	UPROPERTY()
	UTestType1* RequestTestdata1Response;
	UPROPERTY()
	UTestType1* TestCommandUserTypeResponse;

	TUniquePtr<improbable::unreal::callbacks::FScopedViewCallbacks> Callbacks;

	TMap<uint32_t, const FReserveEntityIdResultDelegate> RequestIdToReserveEntityIdCallback;
	void OnReserveEntityIdResponseDispatcherCallback(const worker::ReserveEntityIdResponseOp& op);

	TMap<uint32_t, const FReserveEntityIdsResultDelegate> RequestIdToReserveEntityIdsCallback;
	void OnReserveEntityIdsResponseDispatcherCallback(const worker::ReserveEntityIdsResponseOp& op);

    TMap<uint32_t, const FCreateEntityResultDelegate> RequestIdToCreateEntityCallback;
	void OnCreateEntityResponseDispatcherCallback(const worker::CreateEntityResponseOp& op);

    TMap<uint32_t, const FDeleteEntityResultDelegate> RequestIdToDeleteEntityCallback;
	void OnDeleteEntityResponseDispatcherCallback(const worker::DeleteEntityResponseOp& op);

	TMap<uint32_t, FEntityQueryCountResultDelegate> RequestIdToEntityQueryCountCommandCallback;
	TMap<uint32_t, FEntityQuerySnapshotResultDelegate> RequestIdToEntityQuerySnapshotCommandCallback;
	void OnEntityQueryCommandResponseDispatcherCallback(const worker::EntityQueryResponseOp& op);

	TMap<uint32_t, const FRequestTestdata2CommandResultDelegate> RequestIdToRequestTestdata2CommandCallback;
	void OnRequestTestdata2CommandResponseDispatcherCallback(const worker::CommandResponseOp<test::TestData1::Commands::RequestTestdata2>& op);

	TMap<uint32_t, const FRequestTestdata1CommandResultDelegate> RequestIdToRequestTestdata1CommandCallback;
	void OnRequestTestdata1CommandResponseDispatcherCallback(const worker::CommandResponseOp<test::TestData2::Commands::RequestTestdata1>& op);

	TMap<uint32_t, const FTestCommandUserTypeCommandResultDelegate> RequestIdToTestCommandUserTypeCommandCallback;
	void OnTestCommandUserTypeCommandResponseDispatcherCallback(const worker::CommandResponseOp<test::TestCommandResponseTypes::Commands::TestCommandUserType>& op);

	TMap<uint32_t, const FTestCommandCoordinatesCommandResultDelegate> RequestIdToTestCommandCoordinatesCommandCallback;
	void OnTestCommandCoordinatesCommandResponseDispatcherCallback(const worker::CommandResponseOp<test::TestCommandResponseTypes::Commands::TestCommandCoordinates>& op);

	TMap<uint32_t, const FTestCommandVector3dCommandResultDelegate> RequestIdToTestCommandVector3dCommandCallback;
	void OnTestCommandVector3dCommandResponseDispatcherCallback(const worker::CommandResponseOp<test::TestCommandResponseTypes::Commands::TestCommandVector3d>& op);

	TMap<uint32_t, const FTestCommandVector3fCommandResultDelegate> RequestIdToTestCommandVector3fCommandCallback;
	void OnTestCommandVector3fCommandResponseDispatcherCallback(const worker::CommandResponseOp<test::TestCommandResponseTypes::Commands::TestCommandVector3f>& op);

};
