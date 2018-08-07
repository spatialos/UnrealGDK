// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorInteropCodeGenerator.h"

#include "GenericPlatform/GenericPlatformFile.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "SchemaGenerator.h"
#include "TypeBindingGenerator.h"
#include "TypeStructure.h"
#include "SpatialGDKEditorUtils.h"
#include "SpatialGDKEditorToolbarSettings.h"
#include "Utils/CodeWriter.h"
#include "Utils/ComponentIdGenerator.h"
#include "Utils/DataTypeUtilities.h"

#include "Misc/MonitoredProcess.h"
#include "SharedPointer.h"

#include "Misc/FileHelper.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKInteropCodeGenerator);

namespace
{

void OnStatusOutput(FString Message)
{
	UE_LOG(LogSpatialGDKInteropCodeGenerator, Log, TEXT("%s"), *Message);
}

int GenerateCompleteSchemaFromClass(const FString& SchemaPath, const FString& ForwardingCodePath, int ComponentId, UClass* Class, const TArray<FString>& TypeBindingHeaders, bool bIsSingleton, const ClassHeaderMap& InteropGeneratedClasses)
{
	FCodeWriter OutputSchema;
	FCodeWriter OutputHeader;
	FCodeWriter OutputSource;

	FString SchemaFilename = FString::Printf(TEXT("Unreal%s"), *UnrealNameToSchemaTypeName(Class->GetName()));
	FString TypeBindingFilename = FString::Printf(TEXT("SpatialTypeBinding_%s"), *Class->GetName());

	// Parent and static array index start at 0 for checksum calculations.
	TSharedPtr<FUnrealType> TypeInfo = CreateUnrealTypeInfo(Class, 0, 0, false);

	// Generate schema.
	int NumComponents = GenerateTypeBindingSchema(OutputSchema, ComponentId, Class, TypeInfo, SchemaPath);
	OutputSchema.WriteToFile(FString::Printf(TEXT("%s%s.schema"), *SchemaPath, *SchemaFilename));

	// Generate forwarding code.
	GenerateTypeBindingHeader(OutputHeader, SchemaFilename, TypeBindingFilename, Class, TypeInfo);
	GenerateTypeBindingSource(OutputSource, SchemaFilename, TypeBindingFilename, Class, TypeInfo, TypeBindingHeaders, bIsSingleton, InteropGeneratedClasses);
	OutputHeader.WriteToFile(FString::Printf(TEXT("%s%s.h"), *ForwardingCodePath, *TypeBindingFilename));
	OutputSource.WriteToFile(FString::Printf(TEXT("%s%s.cpp"), *ForwardingCodePath, *TypeBindingFilename));

	return NumComponents;
}

bool CheckClassNameListValidity(const ClassHeaderMap& Classes)
{
	// Pull out all the class names from the map. (These might contain underscores like "One_TwoThree" and "OneTwo_Three").
	TArray<UClass*> ClassNames;
	Classes.GetKeys(ClassNames);

	// Remove all underscores from the class names, check for duplicates.
	for (int i = 0; i < ClassNames.Num() - 1; ++i)
	{
		const FString& ClassA = ClassNames[i]->GetName();
		const FString SchemaTypeA = UnrealNameToSchemaTypeName(ClassA);

		for (int j = i + 1; j < ClassNames.Num(); ++j)
		{
			const FString& ClassB = ClassNames[j]->GetName();
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

TArray<FString> CreateSingletonListFromConfigFile()
{
	TArray<FString> SingletonList;

	const FString FileName = "DefaultEditorSpatialGDK.ini";
	const FString ConfigFilePath = FPaths::SourceConfigDir().Append(FileName);

	// Load the SpatialGDK config file
	const FConfigFile* ConfigFile = LoadConfigFile(ConfigFilePath);
	if (!ConfigFile)
	{
		return SingletonList;
	}

	const FString SectionName = "SnapshotGenerator.SingletonActorClasses";
	const FConfigSection* SingletonActorClassesSection = GetConfigSection(ConfigFilePath, SectionName);
	if (SingletonActorClassesSection == nullptr)
	{
		return SingletonList;
	}

	TArray<FName> SingletonActorClasses;
	SingletonActorClassesSection->GetKeys(SingletonActorClasses);

	for (FName ClassName : SingletonActorClasses)
	{
		SingletonList.Add(ClassName.ToString());
	}

	return SingletonList;
}

void GenerateInteropFromClasses(const ClassHeaderMap& Classes, const FString& CombinedSchemaPath, const FString& CombinedForwardingCodePath)
{
	TArray<FString> SingletonList = CreateSingletonListFromConfigFile();

	// Component IDs 100000 to 100009 reserved for other SpatialGDK components.
	int ComponentId = 100010;
	for (auto& ClassHeaderList : Classes)
	{
		const TArray<FString>& TypeBindingHeaders = ClassHeaderList.Value;
		bool bIsSingleton = SingletonList.Find(ClassHeaderList.Key->GetPathName()) != INDEX_NONE;

		ComponentId += GenerateCompleteSchemaFromClass(CombinedSchemaPath, CombinedForwardingCodePath, ComponentId, ClassHeaderList.Key, TypeBindingHeaders, bIsSingleton, Classes);
	}
}

bool RunProcess(const FString& ExecutablePath, const FString& Arguments)
{
	TSharedPtr<FMonitoredProcess> Process = MakeShareable(new FMonitoredProcess(ExecutablePath, Arguments, true));
	Process->OnOutput().BindStatic(&OnStatusOutput);
	Process->Launch();
	// We currently spin on the thread calling this function as this appears to be
	// The idiomatic way according to the other usages of the FMonitoredProcess interface in the Unreal engine 
	// codebase. See TargetPlatformManagerModule.cpp for another example of this setup.
	while (Process->Update())
	{
		FPlatformProcess::Sleep(0.01f);
	}

	if (Process->GetReturnCode() != 0)
	{
		return false;
	}

	return true;
}

FString GenerateIntermediateDirectory()
{
	const FString CombinedIntermediatePath = FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("Intermediate/Improbable/"), *FGuid::NewGuid().ToString(), TEXT("/"));
	FString AbsoluteCombinedIntermediatePath = FPaths::ConvertRelativePathToFull(CombinedIntermediatePath);
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*AbsoluteCombinedIntermediatePath);

	return AbsoluteCombinedIntermediatePath;
}

bool SpatialGDKGenerateInteropCode()
{
	const USpatialGDKEditorToolbarSettings* SpatialGDKToolbarSettings = GetDefault<USpatialGDKEditorToolbarSettings>();
	if (!SpatialGDKToolbarSettings)
	{
		UE_LOG(LogSpatialGDKInteropCodeGenerator, Error, TEXT("No SpatialGDKEditorToolbarSettings available. Ensure that you have the SpatialGDK editor plugin setup correctly."));
		return false;
	}

	ClassHeaderMap InteropGeneratedClasses;
	for (auto& element : SpatialGDKToolbarSettings->InteropCodegenClasses)
	{
		InteropGeneratedClasses.Add(element.Actor, element.IncludeList);
	}


	if (!CheckClassNameListValidity(InteropGeneratedClasses))
	{
		return false;
	}

	FString InteropOutputPath = FPaths::ConvertRelativePathToFull(SpatialGDKToolbarSettings->InteropCodegenOutputFolder.Path);
	FString SchemaOutputPath = FPaths::ConvertRelativePathToFull(SpatialGDKToolbarSettings->GeneratedSchemaOutputFolder.Path);

	UE_LOG(LogSpatialGDKInteropCodeGenerator, Display, TEXT("Schema path %s - Forwarding code path %s"), *SchemaOutputPath, *InteropOutputPath);

	// Check schema path is valid.
	if (!FPaths::CollapseRelativeDirectories(SchemaOutputPath))
	{
		UE_LOG(LogSpatialGDKInteropCodeGenerator, Error, TEXT("Invalid path: '%s'. Schema not generated."), *SchemaOutputPath);
		return false;
	}

	if (!FPaths::CollapseRelativeDirectories(InteropOutputPath))
	{
		UE_LOG(LogSpatialGDKInteropCodeGenerator, Error, TEXT("Invalid path: '%s'. schema not generated."), *InteropOutputPath);
		return false;
	}

	const FString SchemaIntermediatePath = GenerateIntermediateDirectory();
	const FString InteropIntermediatePath = GenerateIntermediateDirectory();
	GenerateInteropFromClasses(InteropGeneratedClasses, SchemaIntermediatePath, InteropIntermediatePath);

	const FString DiffCopyPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("Scripts/DiffCopy.bat")));
	// Copy Interop files.
	FString DiffCopyArguments = FString::Printf(TEXT("\"%s\" \"%s\" --verbose --remove-input"), *InteropIntermediatePath, *InteropOutputPath);
	if (!RunProcess(DiffCopyPath, DiffCopyArguments))
	{
		UE_LOG(LogSpatialGDKInteropCodeGenerator, Error, TEXT("Could not move generated interop files during the diff-copy stage. Path: '%s', arguments: '%s'."), *DiffCopyPath, *DiffCopyArguments);
		return false;
	}

	// Copy schema files
	DiffCopyArguments = FString::Printf(TEXT("\"%s\" \"%s\" --verbose --remove-input"), *SchemaIntermediatePath, *SchemaOutputPath);
	if (!RunProcess(DiffCopyPath, DiffCopyArguments))
	{
		UE_LOG(LogSpatialGDKInteropCodeGenerator, Error, TEXT("Could not move generated schema files during the diff-copy stage. Path: '%s', arguments: '%s'."), *DiffCopyPath, *DiffCopyArguments);
		return false;
	}

	// Run Codegen
	const FString CodegenPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("Scripts/Codegen.bat")));
	if (!RunProcess(CodegenPath, TEXT("")))
	{
		UE_LOG(LogSpatialGDKInteropCodeGenerator, Error, TEXT("Spatial C++ Worker Codegen failed. Path: '%s'."), *CodegenPath);
		return false;
	}

	return true;
}
