#include "EventLogger.h"

uint32 countLogs = 0;

GDKStructuredEventLogger::GDKStructuredEventLogger(FString LogFilePrefix, FString WorkerId, FString WorkerType, const TFunction<uint32()>& LoadbalancingWorkerIdGetter)
	: LocalWorkerId(WorkerId),
	LogDirectory(LogFilePrefix),
	LocalWorkerType(WorkerType),
	LocalVirtualWorkerIdGetter(LoadbalancingWorkerIdGetter)
{
	//LogDirectory = LogFilePrefix;
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
	FString FileName = LogDirectory + LocalWorkerId + TEXT("-");
	FileName.AppendInt(countLogs++);
	FileName.Append(TEXT(".log"));
	
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if(PlatformFile.CreateDirectoryTree(*LogDirectory))
	{
		IFileHandle* FileHandle = PlatformFile.OpenWrite(*FileName, true);
		if(FileHandle != nullptr)
		{
			WriteHandle.Reset(FileHandle);
		}
	}
	
	//WriteStream->open(*LogDirectory, std::ios::app);
}
void GDKStructuredEventLogger::End()
{
	WriteHandle->Flush();
	//WriteStream->close();
}

void GDKStructuredEventLogger::Log(FString& JsonString)
{
	if(!WriteHandle.Get())
	{
		return;
	}
	
	JsonString.Append("\n");
	WriteHandle->Write((const uint8*)TCHAR_TO_ANSI(*JsonString), JsonString.Len());
	//*WriteStream << *JsonString << std::endl;
}
