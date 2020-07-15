#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LogEvent.h"
#include "WorkerSDK/improbable/c_worker.h"
#include "Schema/RPCPayload.h"
#include "Templates/SharedPointer.h"
#include "NetworkEvents.generated.h"

USTRUCT()
struct FNetworkEvent : public FLogEvent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FNetworkData Network;
};

USTRUCT()
struct FAddEntityEvent : public FNetworkEvent
{
	GENERATED_BODY()

public:
	FAddEntityEvent()
	{
		EventName = TEXT("add_entity");
	}
	
	UPROPERTY()
	FActorData Actor;
};

USTRUCT()
struct FRemoveEntityEvent : public FNetworkEvent
{
	GENERATED_BODY()

public:
	FRemoveEntityEvent()
	{
		EventName = TEXT("remove_entity");
	}
	
	UPROPERTY()
	FActorData Actor;
};

USTRUCT()
struct FAddComponentEvent : public FNetworkEvent
{
	GENERATED_BODY()

public:
	FAddComponentEvent()
	{
		EventName = TEXT("add_component");
	}
	
	UPROPERTY()
	FActorData Entity;

	UPROPERTY()
	FComponentData Component;
};

USTRUCT()
struct FRemoveComponentEvent : public FNetworkEvent
{
	GENERATED_BODY()

public:
	FRemoveComponentEvent()
	{
		EventName = TEXT("remove_component");
	}

	UPROPERTY()
	FActorData Entity;

	UPROPERTY()
	FComponentData Component;
};

USTRUCT()
struct FUpdateComponentEvent : public FNetworkEvent
{
	GENERATED_BODY()

public:
	FUpdateComponentEvent()
	{
		EventName = TEXT("update_component");
	}

	UPROPERTY()
	FActorData Actor;

	UPROPERTY()
	FSubobjectData Subobject;

	UPROPERTY()
	FComponentData Component;
};

USTRUCT()
struct FAuthorityChangeEvent : public FNetworkEvent
{
	GENERATED_BODY()

public:
	FAuthorityChangeEvent()
	{
		EventName = TEXT("authority_change");
	}

	UPROPERTY()
	FActorData Actor;

	/*UPROPERTY()
	FComponentData Component;*/

	UPROPERTY()
	FString Authority;
};

USTRUCT()
struct FAuthorityIntentChangeEvent : public FNetworkEvent
{
	GENERATED_BODY()

public:
	FAuthorityIntentChangeEvent()
	{
		EventName = TEXT("authority_intent_change");
	}

	UPROPERTY()
	FActorData Actor;

	UPROPERTY()
	uint32 NewIntendedAuthority;
};

USTRUCT()
struct FRPCRequestEvent : public FNetworkEvent
{
	GENERATED_BODY()

public:
	FRPCRequestEvent()
	{
		EventName = TEXT("rpc_request");
	}

	UPROPERTY()
	FActorData Actor;

	UPROPERTY()
	FSubobjectData Subobject;

	UPROPERTY()
	FRPCData Rpc;
};

USTRUCT()
struct FRPCResponseEvent : public FNetworkEvent
{
	GENERATED_BODY()

public:
	FRPCResponseEvent()
	{
		EventName = TEXT("command_response");
	}

	UPROPERTY()
	FActorData Actor;

	UPROPERTY()
	FSubobjectData Subobject;

	UPROPERTY()
	FRPCData Rpc;
};
