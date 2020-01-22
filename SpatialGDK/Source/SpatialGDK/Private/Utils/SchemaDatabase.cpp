// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SchemaDatabase.h"
#include "Utils/ComponentIdGenerator.h"
#include "Settings/ProjectPackagingSettings.h"
#include "SpatialGDKSettings.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"

FActorSpecificSubobjectSchemaData FActorSpecificSubobjectSchemaData::Generate(FComponentIdGenerator& IdGenerator, const UClass* Class)
{
	//FUnrealFlatRepData RepData = GetFlatRepData(TypeInfo);

	FActorSpecificSubobjectSchemaData SubobjectData;
	SubobjectData.ClassPath = Class->GetPathName();

	for (uint32 i = 0; i<SCHEMA_Count; ++i)
	{
		Worker_ComponentId ComponentId = 0;
		ComponentId = IdGenerator.Next();
		SubobjectData.SchemaComponents[i] = ComponentId;
	}

	return SubobjectData;
}
TMap<uint32, UClass*> GetAllSubobjects(UClass* Class)
{
	uint32 Offset = 1;

	TSet<UObject*> SeenComponent;
	TMap<uint32, UClass*> Subobjects;

	// Iterate through each property in the struct.
	for (TFieldIterator<UProperty> It(Class); It; ++It)
	{
		UProperty* Property = *It;

		UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Property);
		if (!ObjectProperty)
		{
			continue;
		}
		UObject* ContainerCDO = Class->GetDefaultObject();
		check(ContainerCDO);

		// Obtain the properties actual value from the CDO, so we can figure out its true type.
		UObject* Value = ObjectProperty->GetPropertyValue_InContainer(ContainerCDO);
		if (Value)
		{
			// If this is an editor-only property, skip it. As we've already added to the property list at this stage, just remove it.
			if (Value->IsEditorOnly())
			{
				continue;
			}

			// Check whether the outer is the CDO of the class we're generating for
			// or the CDO of any of its parent classes.
			// (this also covers generating schema for a Blueprint derived from the outer's class)
			UObject* Outer = Value->GetOuter();
			if ((Outer != nullptr) &&
				Outer->HasAnyFlags(RF_ClassDefaultObject) &&
				ContainerCDO->IsA(Outer->GetClass()))
			{
				for (int i = 0; i < Property->ArrayDim; i++)
				{
					if (!SeenComponent.Contains(Value))
					{
						Subobjects.Add(Offset, Value->GetClass());
						SeenComponent.Add(Value);
					}
					++Offset;
				}
				continue;
			}
		}

		UClass* BlueprintClass = Class;
		while (UBlueprintGeneratedClass* BGC = Cast<UBlueprintGeneratedClass>(BlueprintClass))
		{
			if (USimpleConstructionScript* SCS = BGC->SimpleConstructionScript)
			{
				for (USCS_Node* Node : SCS->GetAllNodes())
				{
					if (Node->ComponentTemplate == nullptr)
					{
						continue;
					}

					if (ObjectProperty->GetName().Equals(Node->GetVariableName().ToString()))
					{
						for (int i = 0; i < Property->ArrayDim; i++)
						{
							Value = Node->ComponentTemplate;

							if (!SeenComponent.Contains(Value))
							{
								Subobjects.Add(Offset, Value->GetClass());
								SeenComponent.Add(Value);
							}
							++Offset;
						}
					}
				}
			}

			BlueprintClass = BlueprintClass->GetSuperClass();
		}
	} // END TFieldIterator<UProperty>

	return Subobjects;
}

FActorSchemaData FActorSchemaData::Generate(FComponentIdGenerator& IdGenerator, const UClass* Class)
{
	FActorSchemaData ActorSchemaData;
	//ActorSchemaData.GeneratedSchemaName = ClassPathToSchemaName[Class->GetPathName()];

	for (uint32 i = 0; i < SCHEMA_Count; ++i)
	{
		Worker_ComponentId ComponentId = 0;
		ComponentId = IdGenerator.Next();
		ActorSchemaData.SchemaComponents[i] = ComponentId;
	}
	
	auto Subobjects = GetAllSubobjects(const_cast<UClass*>(Class));

	for (auto& It : Subobjects)
	{
		UClass* SubobjectClass = It.Value;

		FActorSpecificSubobjectSchemaData SubobjectData;
		const FActorSpecificSubobjectSchemaData* ExistingSubobjectSchemaData = nullptr;
		
		SubobjectData = FActorSpecificSubobjectSchemaData::Generate(IdGenerator, SubobjectClass);

		//SubobjectData.Name = SubobjectTypeInfo->Name;
		uint32 SubobjectOffset = SubobjectData.SchemaComponents[SCHEMA_Data];
		check(SubobjectOffset != 0);
		ActorSchemaData.SubobjectData.Add(SubobjectOffset, SubobjectData);
	}

	return ActorSchemaData;
}

FSubobjectSchemaData FSubobjectSchemaData::Generate(FComponentIdGenerator& IdGenerator, const UClass* Class)
{
	// Use the max number of dynamically attached subobjects per class to generate
	// that many schema components for this subobject.
	const uint32 DynamicComponentsPerClass = GetDefault<USpatialGDKSettings>()->MaxDynamicallyAttachedSubobjectsPerClass;

	FSubobjectSchemaData SubobjectSchemaData;

	for (uint32 i = 1; i <= DynamicComponentsPerClass; i++)
	{
		FDynamicSubobjectSchemaData DynamicSubobjectComponents;

		for (uint32 j = 0; j < SCHEMA_Count; ++j)
		{
			
			Worker_ComponentId ComponentId = 0;
			ComponentId = IdGenerator.Next();
			DynamicSubobjectComponents.SchemaComponents[j] = ComponentId;
		}
		SubobjectSchemaData.DynamicSubobjectComponents.Add(MoveTemp(DynamicSubobjectComponents));
	}

	return SubobjectSchemaData;
}

const FActorSchemaData& USchemaDatabase::GetOrCreateActorSchemaData(const UClass* Class)
{
	FString ClassPathName = Class->GetPathName();
	if (FActorSchemaData* ActorSchemaData = ActorClassPathToSchema.Find(ClassPathName))
	{
		return *ActorSchemaData;
	}

	FComponentIdGenerator Gen(NextAvailableComponentId);

	auto& ActorSchemaData = ActorClassPathToSchema.Add(ClassPathName, FActorSchemaData::Generate(Gen, Class));

	NextAvailableComponentId = Gen.Peek();
	for (int32 Type = SCHEMA_Begin; Type < SCHEMA_Count; Type++)
	{
		ComponentIdToClassPath.Add(ActorSchemaData.SchemaComponents[Type], ClassPathName);
	}

	for (const auto& SubobjectSchemaData : ActorSchemaData.SubobjectData)
	{
		for (int32 Type = SCHEMA_Begin; Type < SCHEMA_Count; Type++)
		{
			ComponentIdToClassPath.Add(SubobjectSchemaData.Value.SchemaComponents[Type], SubobjectSchemaData.Value.ClassPath);
		}
	}

	return ActorSchemaData;
}

const FSubobjectSchemaData& USchemaDatabase::GetOrCreateSubobjectSchemaData(const UClass* Class)
{
	FString ClassPathName = Class->GetPathName();
	if (FSubobjectSchemaData* SubobjectSchemaData = SubobjectClassPathToSchema.Find(ClassPathName))
	{
		return *SubobjectSchemaData;
	}

	FComponentIdGenerator Gen(NextAvailableComponentId);

	auto& SubobjectSchemaData = SubobjectClassPathToSchema.Add(ClassPathName, FSubobjectSchemaData::Generate(Gen, Class));

	NextAvailableComponentId = Gen.Peek();
	for (const auto& DynamicSubobjectData : SubobjectSchemaData.DynamicSubobjectComponents)
	{
		for (int32 Type = SCHEMA_Begin; Type < SCHEMA_Count; Type++)
		{
			ComponentIdToClassPath.Add(DynamicSubobjectData.SchemaComponents[Type], ClassPathName);
		}
	}
	
	return SubobjectSchemaData;
}

uint32 USchemaDatabase::GetOrCreateLevelComponentId(const FString& Level)
{
	if (uint32* ComponentId = LevelPathToComponentId.Find(Level))
	{
		return *ComponentId;
	}

	FComponentIdGenerator Gen(NextAvailableComponentId);

	uint32 ComponentId = Gen.Next();

	LevelPathToComponentId.Add(Level, ComponentId);
	LevelComponentIds.Add(ComponentId);

	NextAvailableComponentId = Gen.Peek();

	return ComponentId;
}
	
void USchemaDatabase::Set(TMap<FString, FActorSchemaData> InActorClassPathToSchema,
	TMap<FString, FSubobjectSchemaData> InSubobjectClassPathToSchema,
	TMap<FString, uint32> InLevelPathToComponentId,
	TMap<uint32, FString> InComponentIdToClassPath,
	TSet<uint32> InLevelComponentIds,
	uint32 InNextAvailableComponentId)
{
	ActorClassPathToSchema = MoveTemp(InActorClassPathToSchema);
	SubobjectClassPathToSchema = MoveTemp(InSubobjectClassPathToSchema);
	LevelPathToComponentId = MoveTemp(InLevelPathToComponentId);
	ComponentIdToClassPath = MoveTemp(InComponentIdToClassPath);
	LevelComponentIds = MoveTemp(InLevelComponentIds);
	NextAvailableComponentId = InNextAvailableComponentId;
}
