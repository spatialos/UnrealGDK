#pragma once

#include <iostream>
#include <fstream>

#include "Runtime/JsonUtilities/Public/JsonUtilities.h"
#include "LogEvent.h"
#include "NetworkEvents.h"

class GDKStructuredEventLogger
{
public:
	GDKStructuredEventLogger(){}
	GDKStructuredEventLogger(FString LogFilePrefix, FString WorkerId, FString WorkerType, uint32 LoadbalancingWorkerid);
	~GDKStructuredEventLogger();

	void operator=(GDKStructuredEventLogger&& other) noexcept
	{
		LogDirectory = other.LogDirectory;
		LocalWorkerId = other.LocalWorkerId;
		LocalWorkerType = other.LocalWorkerType;
		LocalVirtualWorkerId = other.LocalVirtualWorkerId;
		WriteHandle.Reset(other.WriteHandle.Release());
	}

	void Start();
	void End();
	
	template<typename TLogEvent>
	void LogEvent(TLogEvent& Event)
	{
		static_assert(std::is_base_of<FLogEvent, TLogEvent>::value, "TLogEvent must inherit from FLogEvent.");

		Event.Time = FDateTime::UtcNow().ToIso8601();
		Event.WorkerId = LocalWorkerId;
		Event.WorkerType = LocalWorkerType;
		Event.VirtualWorkerId = LocalVirtualWorkerId;
		
		FString JsonEvent;
		TSharedRef<FJsonObject> TopJsonObject = MakeShared<FJsonObject>();
		TopJsonObject->SetObjectField(TEXT("ue4_event"), FJsonObjectConverter::UStructToJsonObject(Event));

		TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>> > JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonEvent, 0);
		bool bSuccess = FJsonSerializer::Serialize(TopJsonObject, JsonWriter);
		JsonWriter->Close();
		
		//FJsonObjectConverter::UStructToJsonObjectString<FTestEventTopLevel>(SerialiseEvent, JsonEvent, 0, 0, 0, nullptr, false);
		Log(JsonEvent);
	}

private:
	//TUniquePtr<std::wofstream> WriteStream;
	TUniquePtr<IFileHandle> WriteHandle;
	
	FString LogDirectory;
	FString LocalWorkerId;
	FString LocalWorkerType;
	uint32 LocalVirtualWorkerId;
	
	void Log(FString& Json);
};
