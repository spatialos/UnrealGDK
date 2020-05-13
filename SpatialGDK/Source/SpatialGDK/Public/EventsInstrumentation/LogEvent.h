#pragma once

#include "CoreMinimal.h"
#include "LogEvent.generated.h"

USTRUCT()
struct FLogEvent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString Time;
	
	UPROPERTY()
	FString WorkerId;

	UPROPERTY()
	FString WorkerType;

	UPROPERTY()
	uint32 VirtualWorkerId;

	UPROPERTY()
	FString EventName;
};

USTRUCT()
struct FNetworkData
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FString MessageDirection;
};

USTRUCT()
struct FActorData
{
	GENERATED_BODY()

public:	
	UPROPERTY()
	uint64 EntityId;

	UPROPERTY()
	FString Type;

	UPROPERTY()
	FString Name;
};

USTRUCT()
struct FSubobjectData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString Type;

	UPROPERTY()
	FString Name;
};

USTRUCT()
struct FComponentData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	uint64 Id;
};

USTRUCT()
struct FRPCData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString Type;

	UPROPERTY()
	uint64 LocalRequestId;

	UPROPERTY()
	int32 TraceKey;
	
	UPROPERTY()
	FString Name;
};
