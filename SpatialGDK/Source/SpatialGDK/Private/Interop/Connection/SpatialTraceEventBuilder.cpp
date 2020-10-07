// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialTraceEventBuilder.h"

#include "Utils/SpatialActorUtils.h"

namespace SpatialGDK
{
FSpatialTraceEventBuilder::FSpatialTraceEventBuilder(const char* InType)
	: SpatialTraceEvent(InType, "")
{
}

FSpatialTraceEventBuilder::FSpatialTraceEventBuilder(const char* InType, FString InMessage)
	: SpatialTraceEvent(InType, MoveTemp(InMessage))
{
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddObject(FString Key, const UObject* Object)
{
	if (Object != nullptr)
	{
		if (const AActor* Actor = Cast<AActor>(Object))
		{
			AddKeyValue(TEXT("ActorPosition"), Actor->GetTransform().GetTranslation().ToString());
		}
		if (UWorld* World = Object->GetWorld())
		{
			if (USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(World->GetNetDriver()))
			{
				AddKeyValue(TEXT("NetGuid"), NetDriver->PackageMap->GetNetGUIDFromObject(Object).ToString());
			}
		}
		AddKeyValue(MoveTemp(Key), Object->GetName());
		
	}
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddFunction(FString Key, const UFunction* Function)
{
	if (Function != nullptr)
	{
		AddKeyValue(MoveTemp(Key), Function->GetName());
	}
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddEntityId(FString Key, const Worker_EntityId EntityId)
{
	AddKeyValue(MoveTemp(Key), FString::FromInt(EntityId));
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddComponentId(FString Key, const Worker_ComponentId ComponentId)
{
	AddKeyValue(MoveTemp(Key), FString::FromInt(ComponentId));
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddFieldId(FString Key, const uint32 FieldId)
{
	AddKeyValue(MoveTemp(Key), FString::FromInt(FieldId));
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddNewWorkerId(FString Key, const uint32 NewWorkerId)
{
	AddKeyValue(MoveTemp(Key), FString::FromInt(NewWorkerId));
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddCommand(FString Key, const FString& Command)
{
	AddKeyValue(MoveTemp(Key), Command);
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddRequestID(FString Key, const int64 RequestID)
{
	AddKeyValue(MoveTemp(Key), FString::FromInt(RequestID));
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddNetRole(FString Key, const ENetRole Role)
{
	AddKeyValue(MoveTemp(Key), NetRoleToString(Role));
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddKeyValue(FString Key, FString Value)
{
	SpatialTraceEvent.AddData(MoveTemp(Key), MoveTemp(Value));
	return *this;
}

FSpatialTraceEvent FSpatialTraceEventBuilder::GetEvent() &&
{
	return MoveTemp(SpatialTraceEvent);
}
} // namespace SpatialGDK
