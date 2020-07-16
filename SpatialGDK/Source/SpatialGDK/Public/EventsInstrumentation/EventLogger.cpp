#include "EventLogger.h"

GDKStructuredEventLogger::GDKStructuredEventLogger(FString LogFilePrefix, FString WorkerId, FString WorkerType, const TFunction<uint32()>& LoadbalancingWorkerIdGetter)
	: LogDirectory(LogFilePrefix),
	LocalWorkerId(WorkerId),
	LocalWorkerType(WorkerType),
	LocalVirtualWorkerIdGetter(LoadbalancingWorkerIdGetter)
{
}

GDKStructuredEventLogger::~GDKStructuredEventLogger()
{
	if(WriteHandle.Get())
	{
		WriteHandle->Flush();
	}
}

void GDKStructuredEventLogger::Start()
{
	FString LogDirectoryPath = FPaths::Combine(FPaths::ProjectLogDir(), LogDirectory);
	
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if(PlatformFile.CreateDirectoryTree(*LogDirectoryPath))
	{
		FString FileName = LocalWorkerId + TEXT(".log");
		FString FilePath = FPaths::Combine(LogDirectoryPath, FileName);
		IFileHandle* FileHandle = PlatformFile.OpenWrite(*FilePath, true);
		if(FileHandle != nullptr)
		{
			WriteHandle.Reset(FileHandle);
		}
	}
}
void GDKStructuredEventLogger::End()
{
	if (!WriteHandle.Get())
	{
		return;
	}
	WriteHandle->Flush();
}

void GDKStructuredEventLogger::Log(FString& JsonString)
{
	if(!WriteHandle.Get())
	{
		return;
	}
	
	JsonString.Append("\n");
	WriteHandle->Write((const uint8*)TCHAR_TO_ANSI(*JsonString), JsonString.Len());
}
