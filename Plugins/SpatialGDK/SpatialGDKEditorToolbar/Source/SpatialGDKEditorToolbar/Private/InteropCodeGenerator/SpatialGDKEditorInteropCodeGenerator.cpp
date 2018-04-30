// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorInteropCodeGenerator.h"

#include "SchemaGenerator.h"
#include "TypeBindingGenerator.h"
#include "TypeStructure.h"
#include "Utils/CodeWriter.h"
#include "Utils/ComponentIdGenerator.h"

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

	TSharedPtr<FUnrealType> TypeInfo = CreateUnrealTypeInfo(Class, MigratableProperties);

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

void GenerateTypeBindingList(const FString& ForwardingCodePath, const TArray<FString>& Classes)
{
	FCodeWriter OutputListHeader;
	FCodeWriter OutputListSource;

	// Header.
	OutputListHeader.Print(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		#pragma once

		TArray<UClass*> GetGeneratedTypeBindings();)""");

	// Implementation.
	OutputListSource.Print(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically

		#include "SpatialTypeBindingList.h")""");
	OutputListSource.PrintNewLine();
	for (auto& ClassName : Classes)
	{
		OutputListSource.Printf("#include \"SpatialTypeBinding_%s.h\"", *ClassName);
	}
	OutputListSource.PrintNewLine();

	// GetGeneratedTypeBindings.
	OutputListSource.BeginFunction({"TArray<UClass*>", "GetGeneratedTypeBindings()"});
	OutputListSource.Print("return {");
	OutputListSource.Indent();
	for (int i = 0; i < Classes.Num(); ++i)
	{
		OutputListSource.Printf(TEXT("USpatialTypeBinding_%s::StaticClass()%s"), *Classes[i], i < (Classes.Num() - 1) ? TEXT(",") : TEXT(""));
	}
	OutputListSource.Outdent();
	OutputListSource.Print("};");
	OutputListSource.End();

	// Write to files.
	OutputListHeader.WriteToFile(FString::Printf(TEXT("%sSpatialTypeBindingList.h"), *ForwardingCodePath));
	OutputListSource.WriteToFile(FString::Printf(TEXT("%sSpatialTypeBindingList.cpp"), *ForwardingCodePath));
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

void SpatialGDKGenerateInteropCode()
{
	FString CombinedSchemaPath = FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("../../../schema/improbable/unreal/generated/"));
	FString CombinedForwardingCodePath = FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("../../../workers/unreal/Game/Source/SampleGame/Generated/"));
	FString AbsoluteCombinedSchemaPath = FPaths::ConvertRelativePathToFull(CombinedSchemaPath);
	FString AbsoluteCombinedForwardingCodePath = FPaths::ConvertRelativePathToFull(CombinedForwardingCodePath);

	UE_LOG(LogSpatialGDKInteropCodeGenerator, Display, TEXT("Schema path %s - Forwarding code path %s"), *AbsoluteCombinedSchemaPath, *AbsoluteCombinedForwardingCodePath);

	// SpatialGDK config file definitions.
	const FString FileName = "DefaultEditorSpatialGDK.ini";
	const FString ConfigFilePath = FPaths::SourceConfigDir().Append(FileName);
	const FString UserClassesSectionName = "UserInteropCodeGenSettings";

	// Load the SpatialGDK config file
	GConfig->LoadFile(ConfigFilePath);
	FConfigFile* SpatialGDKConfigFile = GConfig->Find(ConfigFilePath, false);
	if (!SpatialGDKConfigFile)
	{
		UE_LOG(LogSpatialGDKInteropCodeGenerator, Error, TEXT("Could not open .ini file for class generation: \"%s\""), *ConfigFilePath);
		return;
	}

	// Get the key-value pairs in the InteropCodeGenSection.
	FConfigSection* UserInteropCodeGenSection = SpatialGDKConfigFile->Find(UserClassesSectionName);
	if (!UserInteropCodeGenSection)
	{
		UE_LOG(LogSpatialGDKInteropCodeGenerator, Error, TEXT("Could not find section '%s' in '%s' for class generation, terminating."), *UserClassesSectionName, *ConfigFilePath);
		return;
	}
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

	TMap<FString, TArray<TArray<FName>>> MigratableProperties;
	MigratableProperties.Add("PlayerController", {
		{"AcknowledgedPawn"}
	});
	MigratableProperties.Add("Character", {
		{"CharacterMovement", "GroundMovementMode"},
		{"CharacterMovement", "MovementMode"},
		{"CharacterMovement", "CustomMovementMode"}
	});

	if (!CheckClassNameListValidity(Classes))
	{
		return;
	}

	if (FPaths::CollapseRelativeDirectories(AbsoluteCombinedSchemaPath) && FPaths::CollapseRelativeDirectories(AbsoluteCombinedForwardingCodePath))
	{
		// Component IDs 100000 to 100009 reserved for other SpatialGDK components.
		int ComponentId = 100010;
		for (auto& ClassHeaderList : Classes)
		{
			UClass* Class = FindObject<UClass>(ANY_PACKAGE, *ClassHeaderList.Key);

			// If the class doesn't exist then print an error and carry on.
			if (!Class) 
			{
				UE_LOG(LogSpatialGDKInteropCodeGenerator, Error, TEXT("Could not find unreal class for interop code generation: '%s', skipping."), *ClassHeaderList.Key);
				continue;
			}

			TArray<TArray<FName>> ClassMigratableProperties = MigratableProperties.FindRef(ClassHeaderList.Key);
			const TArray<FString>& TypeBindingHeaders = ClassHeaderList.Value;
			ComponentId += GenerateCompleteSchemaFromClass(CombinedSchemaPath, CombinedForwardingCodePath, ComponentId, Class, ClassMigratableProperties, TypeBindingHeaders);
		}
	}
	else
	{
		UE_LOG(LogSpatialGDKInteropCodeGenerator, Error, TEXT("Path was invalid - schema not generated"));
	}
}
