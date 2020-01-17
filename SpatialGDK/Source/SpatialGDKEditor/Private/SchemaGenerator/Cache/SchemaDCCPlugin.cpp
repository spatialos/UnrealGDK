// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SchemaGenerator/Cache/SchemaDCCPlugin.h"
#include "DerivedDataCache/Public/DerivedDataCacheInterface.h"

#include "SchemaGenerator/SchemaGenerator.h"
#include "SchemaGenerator/Utils/ComponentIdGenerator.h"
#include "SchemaGenerator/Utils/CodeWriter.h"

#include "GameFramework/Actor.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"



FSchemaClassCache::FSchemaClassCache(UClass* Class)
	:ClassToInspect(Class)
{
	check(Class);
}

const TCHAR* FSchemaClassCache::GetPluginName() const
{
	return TEXT("SpatialGDKSchemaClassCache");
}

const TCHAR* FSchemaClassCache::GetVersionString() const
{
	return TEXT("49C64808-8CE2-487C-8596-4D9B2DA57D8C");
}

FString FSchemaClassCache::GetPluginSpecificCacheKeySuffix() const
{
	uint32 ClassHash = GetTypeHash(ClassToInspect);
	FString ClassId;
	ClassToInspect->GetName(ClassId);
	ClassId.Append("-");
	ClassId.Append(FString::FromInt(ClassHash));

	return ClassId;
}

bool FSchemaClassCache::IsBuildThreadsafe() const
{
	// Global maps in the schema generator :/
	return false;
}

bool FSchemaClassCache::IsDeterministic() const
{
	// Modulo the component Id :/
	return true;
}

FString FSchemaClassCache::GetDebugContextString() const
{
	return "";
}
namespace SpatialGDKEditor
{
	namespace Schema
	{
		bool IsSupportedClass(const UClass* SupportedClass);
	}
}

bool FSchemaClassCache::Build(TArray<uint8>& OutData)
{
	OutData.Empty();

	FMemoryWriter Ar(OutData);

	if (SpatialGDKEditor::Schema::IsSupportedClass(ClassToInspect))
	{
		FComponentIdGenerator DummyGenerator(0x70000000);
		TArray<TSharedPtr<FUnrealType>> TypeInfos;
		TSharedPtr<FUnrealType> TypeInfo = CreateUnrealTypeInfo(ClassToInspect, 0, 0); 
		TypeInfos.Add(TypeInfo);
		VisitAllObjects(TypeInfo, [&](TSharedPtr<FUnrealType> TypeNode)
		{
			if (UClass* NestedClass = Cast<UClass>(TypeNode->Type))
			{
				if (!SchemaGeneratedClasses.Contains(NestedClass) && SpatialGDKEditor::Schema::IsSupportedClass(NestedClass))
				{
					TypeInfos.Add(CreateUnrealTypeInfo(NestedClass, 0, 0));
					SchemaGeneratedClasses.Add(NestedClass);
				}
			}
			return true;
		});

		if (ClassToInspect->IsChildOf<AActor>())
		{
			FCodeWriter ActorWriter;
			FCodeWriter SubobjectsWriter;

			GenerateActorSchema(DummyGenerator, ClassToInspect, TypeInfo, ActorWriter, SubobjectsWriter);

			if (!ActorWriter.GetOutput().IsEmpty())
			{
				if (!SubobjectsWriter.GetOutput().IsEmpty())
				{
					OutputType Type = OutputType::ActorAndSubobjects;
					Ar << (uint32&)Type;
				}
				else
				{
					OutputType Type = OutputType::Actor;
					Ar << (uint32&)Type;
				}

				Ar << const_cast<FString&>(ActorWriter.GetOutput());
				if (!SubobjectsWriter.GetOutput().IsEmpty())
				{
					Ar << const_cast<FString&>(SubobjectsWriter.GetOutput());
				}
			}
			else
			{
				OutputType Type = OutputType::Empty;
				Ar << (uint32&)Type;
			}
		}
		else
		{
			FCodeWriter Writer;

			GenerateSubobjectSchema(DummyGenerator, ClassToInspect, TypeInfo, Writer);

			if(!Writer.GetOutput().IsEmpty())
			{
				OutputType Type = OutputType::Subobject;
				Ar << (int32&)Type;
				Ar << const_cast<FString&>(Writer.GetOutput());
			}
			else
			{
				OutputType Type = OutputType::Empty;
				Ar << (uint32&)Type;
			}
		}
	}
	else
	{
		OutputType Type = OutputType::Empty;
		Ar << (uint32&)Type;
	}

	return true;
}

FSchemaClassCache::OutputType FSchemaClassCache::ReadCachedData(const TArray<uint8>& CachedData, FString& OutObject, FString& OutSubobjects)
{
	OutputType CachedDataType;
	FMemoryReader Ar(CachedData);

	Ar << reinterpret_cast<uint32&>(CachedDataType);

	if (CachedDataType != OutputType::Empty)
	{
		Ar << OutObject;
		if (CachedDataType == OutputType::ActorAndSubobjects)
		{
			Ar << OutSubobjects;
		}
	}

	return CachedDataType;
}
