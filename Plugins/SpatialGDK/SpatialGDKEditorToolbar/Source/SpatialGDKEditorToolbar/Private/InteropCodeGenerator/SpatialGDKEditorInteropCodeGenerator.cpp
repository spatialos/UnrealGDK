// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorInteropCodeGenerator.h"

#include "GenericPlatform/GenericPlatformFile.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "SchemaGenerator.h"
#include "TypeBindingGenerator.h"
#include "TypeStructure.h"
#include "Utils/CodeWriter.h"
#include "Utils/ComponentIdGenerator.h"
#include "Utils/DataTypeUtilities.h"

#include "Misc/FileHelper.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKInteropCodeGenerator);

namespace
{

typedef TMap<FString, TArray<FString>> ClassHeaderMap;

int GenerateCompleteSchemaFromClass(const FString& SchemaPath, const FString& ForwardingCodePath, int ComponentId, UClass* Class, const TArray<TArray<FName>>& MigratableProperties, const TArray<FString>& TypeBindingHeaders)
{
	FCodeWriter OutputSchema;
	FCodeWriter OutputHeader;
	FCodeWriter OutputSource;

	FString SchemaFilename = FString::Printf(TEXT("Unreal%s"), *UnrealNameToSchemaTypeName(Class->GetName()));
	FString TypeBindingFilename = FString::Printf(TEXT("SpatialTypeBinding_%s"), *Class->GetName());

	// Parent and static array index start at 0 for checksum calculations.
	TSharedPtr<FUnrealType> TypeInfo = CreateUnrealTypeInfo(Class, MigratableProperties, 0, 0, false);

	// Generate schema.
	int NumComponents = GenerateTypeBindingSchema(OutputSchema, ComponentId, Class, TypeInfo, SchemaPath);
	OutputSchema.WriteToFile(FString::Printf(TEXT("%s%s.schema"), *SchemaPath, *SchemaFilename));

	// Generate forwarding code.
	GenerateTypeBindingHeader(OutputHeader, SchemaFilename, TypeBindingFilename, Class, TypeInfo);
	GenerateTypeBindingSource(OutputSource, SchemaFilename, TypeBindingFilename, Class, TypeInfo, TypeBindingHeaders);
	OutputHeader.WriteToFile(FString::Printf(TEXT("%s%s.h"), *ForwardingCodePath, *TypeBindingFilename));
	OutputSource.WriteToFile(FString::Printf(TEXT("%s%s.cpp"), *ForwardingCodePath, *TypeBindingFilename));

	return NumComponents;
}

bool CheckClassNameListValidity(const ClassHeaderMap& Classes)
{
	// Pull out all the class names from the map. (These might contain underscores like "One_TwoThree" and "OneTwo_Three").
	TArray<FString> ClassNames;
	Classes.GetKeys(ClassNames);

	// Remove all underscores from the class names, check for duplicates.
	for (int i = 0; i < ClassNames.Num() - 1; ++i)
	{
		const FString& ClassA = ClassNames[i];
		const FString SchemaTypeA = UnrealNameToSchemaTypeName(ClassA);

		for (int j = i + 1; j < ClassNames.Num(); ++j)
		{
			const FString& ClassB = ClassNames[j];
			const FString SchemaTypeB = UnrealNameToSchemaTypeName(ClassB);

			if (SchemaTypeA.Equals(SchemaTypeB))
			{
				UE_LOG(LogSpatialGDKInteropCodeGenerator, Error, TEXT("Class name collision after removing underscores: '%s' and '%s' - schema not generated"), *ClassA, *ClassB);
				return false;
			}
		}
	}
	return true;
}
}// ::

const FConfigFile* GetConfigFile(const FString& ConfigFilePath)
{
	const FConfigFile* ConfigFile = GConfig->Find(ConfigFilePath, false);
	if (!ConfigFile)
	{
		UE_LOG(LogSpatialGDKInteropCodeGenerator, Error, TEXT("Could not open .ini file: \"%s\""), *ConfigFilePath);
		return nullptr;
	}
	return ConfigFile;
}

const FConfigSection* GetConfigSection(const FString& ConfigFilePath, const FString& SectionName)
{
	if (const FConfigFile* ConfigFile = GetConfigFile(ConfigFilePath))
	{
		// Get the key-value pairs in the InteropCodeGenSection.
		if (const FConfigSection* Section = ConfigFile->Find(SectionName))
		{
			return Section;
		}
		else
		{
			UE_LOG(LogSpatialGDKInteropCodeGenerator, Error, TEXT("Could not find section '%s' in '%s'."), *SectionName, *ConfigFilePath);
		}
	}
	return nullptr;
}

const ClassHeaderMap GenerateClassHeaderMap(const FConfigSection* UserInteropCodeGenSection)
{
	TArray<FName> AllCodeGenKeys;
	UserInteropCodeGenSection->GetKeys(AllCodeGenKeys);
	ClassHeaderMap Classes;

	// Iterate over the keys (class names) and extract header includes.
	for (FName ClassKey : AllCodeGenKeys)
	{
		TArray<FString> HeaderValueArray;
		UserInteropCodeGenSection->MultiFind(ClassKey, HeaderValueArray);
		Classes.Add(ClassKey.ToString(), HeaderValueArray);

		// Just for some user facing logging.
		FString Headers;
		for (FString& Header : HeaderValueArray)
		{
			Headers.Append(FString::Printf(TEXT("\"%s\" "), *Header));
		}
		UE_LOG(LogSpatialGDKInteropCodeGenerator, Log, TEXT("Found class to generate interop code for: '%s', with includes %s"), *ClassKey.GetPlainNameString(), *Headers);
	}
	return Classes;
}

const FString GetOutputPath(const FString& ConfigFilePath)
{
	FString OutputPath = FString::Printf(TEXT("%s/Generated/"), FApp::GetProjectName());
	const FString SettingsSectionName = "InteropCodeGen.Settings";
	if (const FConfigSection* SettingsSection = GetConfigSection(ConfigFilePath, SettingsSectionName))
	{
		if (const FConfigValue* OutputModuleSetting = SettingsSection->Find("OutputPath"))
		{
			OutputPath = OutputModuleSetting->GetValue();
		}
	}

	// Ensure that the specified path ends with a path separator.
	OutputPath.AppendChar('/');

	return OutputPath;
}

const bool ClassesExist(const ClassHeaderMap& Classes)
{
	for (const auto& ClassHeaderList : Classes)
	{
		const UClass* Class = FindObject<UClass>(ANY_PACKAGE, *ClassHeaderList.Key);

		// If the class doesn't exist then print an error and carry on.
		if (!Class)
		{
			UE_LOG(LogSpatialGDKInteropCodeGenerator, Error, TEXT("Could not find unreal class for interop code generation: '%s', terminating."), *ClassHeaderList.Key);
			return false;
		}
	}

	return true;
}

void GenerateInteropFromClasses(const ClassHeaderMap& Classes, const FString& CombinedSchemaPath, const FString& CombinedForwardingCodePath)
{
	TMap<FString, TArray<TArray<FName>>> MigratableProperties;
	MigratableProperties.Add("PlayerController", {
		{ "AcknowledgedPawn" }
	});
	MigratableProperties.Add("Character", {
		{ "CharacterMovement", "GroundMovementMode" },
		{ "CharacterMovement", "MovementMode" },
		{ "CharacterMovement", "CustomMovementMode" }
	});

	// Component IDs 100000 to 100009 reserved for other SpatialGDK components.
	int ComponentId = 100010;
	for (auto& ClassHeaderList : Classes)
	{
		UClass* Class = FindObject<UClass>(ANY_PACKAGE, *ClassHeaderList.Key);

		TArray<TArray<FName>> ClassMigratableProperties = MigratableProperties.FindRef(ClassHeaderList.Key);
		const TArray<FString>& TypeBindingHeaders = ClassHeaderList.Value;
		ComponentId += GenerateCompleteSchemaFromClass(CombinedSchemaPath, CombinedForwardingCodePath, ComponentId, Class, ClassMigratableProperties, TypeBindingHeaders);
	}
}

bool RunProcess(const FString& Command, const FString& Arguments)
{
	int32 ReturnCode = 0;
	FString StandardOutput;

	void* ReadPipe = nullptr;
	void* WritePipe = nullptr;
	FPlatformProcess::CreatePipe(ReadPipe, WritePipe);
	FProcHandle ProcHandle = FPlatformProcess::CreateProc(*Command, *Arguments, false, true, true, nullptr, 0, nullptr, WritePipe, ReadPipe);

	if (ProcHandle.IsValid())
	{
		FPlatformProcess::WaitForProc(ProcHandle);
		StandardOutput = FPlatformProcess::ReadPipe(ReadPipe);
		FPlatformProcess::GetProcReturnCode(ProcHandle, &ReturnCode);
	}

	FPlatformProcess::ClosePipe(ReadPipe, WritePipe);
	FPlatformProcess::CloseProc(ProcHandle);

	if (ReturnCode != 0)
	{
		UE_LOG(LogSpatialGDKInteropCodeGenerator, Error, TEXT("%s"), *StandardOutput);
		return false;
	}

	UE_LOG(LogSpatialGDKInteropCodeGenerator, Display, TEXT("%s"), *StandardOutput);
	return true;
}

bool SpatialGDKGenerateInteropCode()
{
	// SpatialGDK config file definitions.
	const FString FileName = "DefaultEditorSpatialGDK.ini";
	const FString ConfigFilePath = FPaths::SourceConfigDir().Append(FileName);
	// Load the SpatialGDK config file
	GConfig->LoadFile(ConfigFilePath);

	const FString UserClassesSectionName = "InteropCodeGen.ClassesToGenerate";
	if (const FConfigSection* UserInteropCodeGenSection = GetConfigSection(ConfigFilePath, UserClassesSectionName))
	{
		const ClassHeaderMap Classes = GenerateClassHeaderMap(UserInteropCodeGenSection);
		if (!CheckClassNameListValidity(Classes))
		{
			return false;
		}

		if (!ClassesExist(Classes))
		{
			return false;
		}

		const FString CombinedSchemaIntermediatePath = FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("Intermediate/Improbable/"), *FGuid::NewGuid().ToString(), TEXT("/"));
		FString AbsoluteCombinedSchemaIntermediatePath = FPaths::ConvertRelativePathToFull(CombinedSchemaIntermediatePath);
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*AbsoluteCombinedSchemaIntermediatePath);

		const FString CombinedSchemaPath = FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("../spatial/schema/improbable/unreal/generated/"));
		FString AbsoluteCombinedSchemaPath = FPaths::ConvertRelativePathToFull(CombinedSchemaPath);

		const FString CombinedIntermediatePath = FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("Intermediate/Improbable/"), *FGuid::NewGuid().ToString(), TEXT("/"));
		FString AbsoluteCombinedIntermediatePath = FPaths::ConvertRelativePathToFull(CombinedIntermediatePath);
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*AbsoluteCombinedIntermediatePath);

		const FString CombinedForwardingCodePath = FPaths::Combine(*FPaths::GetPath(FPaths::GameSourceDir()), *GetOutputPath(ConfigFilePath));
		FString AbsoluteCombinedForwardingCodePath = FPaths::ConvertRelativePathToFull(CombinedForwardingCodePath);

		UE_LOG(LogSpatialGDKInteropCodeGenerator, Display, TEXT("Schema path %s - Forwarding code path %s"), *AbsoluteCombinedSchemaPath, *AbsoluteCombinedForwardingCodePath);

		if (FPaths::CollapseRelativeDirectories(AbsoluteCombinedSchemaPath) && FPaths::CollapseRelativeDirectories(AbsoluteCombinedForwardingCodePath))
		{
			GenerateInteropFromClasses(Classes, AbsoluteCombinedSchemaIntermediatePath, AbsoluteCombinedIntermediatePath);

			const FString DiffCopyPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("Scripts/DiffCopy.bat")));

			// Copy Interop files.
			FString DiffCopyArguments = FString::Printf(TEXT("\"%s\" \"%s\" --verbose"), *AbsoluteCombinedIntermediatePath, *AbsoluteCombinedForwardingCodePath);
			bool bSuccess = RunProcess(DiffCopyPath, DiffCopyArguments);			
			
			if (!bSuccess)
			{
				return false;
			}

			// Copy schema files
			DiffCopyArguments = FString::Printf(TEXT("\"%s\" \"%s\" --verbose"), *AbsoluteCombinedSchemaIntermediatePath, *AbsoluteCombinedSchemaPath);
			bSuccess = RunProcess(DiffCopyPath, DiffCopyArguments);

			if (!bSuccess)
			{
				return false;
			}
			
			// Run Codegen
			const FString CodegenPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("Scripts/Codegen.bat")));
			bSuccess = RunProcess(CodegenPath, TEXT(""));

			return bSuccess;
		}
		else
		{
			UE_LOG(LogSpatialGDKInteropCodeGenerator, Error, TEXT("Path was invalid - schema not generated"));
			return false;
		}
	}

	return false;
}
