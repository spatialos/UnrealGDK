using System.Collections.Generic;
using Improbable.CodeGeneration.Model;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;
using NUnit.Framework;

namespace Improbable.Unreal.CodeGeneration.Test.SchemaProcessing.Unreal.TypeReferences
{
    public class UnrealMapTypeReferenceTest
    {
        [Test]
        public static void unrealmaptypereference_is_initiated_as_expected_when_passed_a_map_type_reference_with_built_in_types_as_contained_types()
        {
            var mapTypeRaw = new FieldDefinitionRaw.MapTypeRaw()
            {
                keyType = new TypeReferenceRaw()
                {
                    sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                    builtInType = "float",
                    userType = null
                },
                valueType = new TypeReferenceRaw()
                {
                    sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                    builtInType = "float",
                    userType = null
                }
            };

            var containedKeyTypeReference = new UnrealBuiltInTypeReference(mapTypeRaw.keyType);
            var containedValueTypeReference = new UnrealBuiltInTypeReference(mapTypeRaw.valueType);
            var unrealMapTypeReference = new UnrealMapTypeReference(containedKeyTypeReference, containedValueTypeReference);

            Assert.That(unrealMapTypeReference.UnderlyingCapitalisedName == null);
            Assert.That(unrealMapTypeReference.RequiredIncludes.Count == 1);
            Assert.That(unrealMapTypeReference.RequiredIncludes.Contains("\"FloatToFloatMap.h\""));
            Assert.That(unrealMapTypeReference.UnderlyingQualifiedName == "worker::Map<float, float>");
            Assert.That(unrealMapTypeReference.UnrealType == "UFloatToFloatMap*");

            Assert.That(unrealMapTypeReference.AssignUnderlyingValueToUnrealMemberVariable("TestField", "val") == "if (TestField == nullptr) { TestField = NewObject<UFloatToFloatMap>(this); } TestField->Init(val)");
            Assert.That(unrealMapTypeReference.ConvertUnderlyingValueToUnrealLocalVariable("val") == "NewObject<UFloatToFloatMap>()->Init(val)");
            Assert.That(unrealMapTypeReference.ConvertUnderlyingValueToUnrealMemberVariable("val") == "NewObject<UFloatToFloatMap>(this)->Init(val)");
            Assert.That(unrealMapTypeReference.ConvertUnrealValueToSnapshotValue("val") == "val->GetUnderlying()");
            Assert.That(unrealMapTypeReference.ConvertUnrealValueToUnderlyingValue("val") == "val->GetUnderlying()");
            Assert.That(unrealMapTypeReference.DefaultInitialisationString == "worker::Map<float, float>()");
            Assert.That(unrealMapTypeReference.SnapshotType == "worker::Map<float, float>");
        }

        [Test]
        public static void unrealmaptypereference_is_initiated_as_expected_when_passed_a_map_type_reference_with_user_types_as_contained_types()
        {
            var userTypeDefinition = GenerateTypeDefinition();

            var unrealTypeDetails = new UnrealTypeDetails(userTypeDefinition, "TestType", new List<UnrealFieldDetails>(), null);
            var containedKeyTypeReference = new UnrealUserTypeReference(unrealTypeDetails);
            var containedValueTypeReference = new UnrealUserTypeReference(unrealTypeDetails);
            var unrealMapTypeReference = new UnrealMapTypeReference(containedKeyTypeReference, containedValueTypeReference);

            Assert.That(unrealMapTypeReference.UnderlyingCapitalisedName == null);
            Assert.That(unrealMapTypeReference.RequiredIncludes.Count == 2);
            Assert.That(unrealMapTypeReference.RequiredIncludes.Contains("\"ImprobableCodegenTestTypeToImprobableCodegenTestTypeMap.h\""));
            Assert.That(unrealMapTypeReference.RequiredIncludes.Contains("\"TestType.h\""));
            Assert.That(unrealMapTypeReference.UnderlyingQualifiedName == "worker::Map<improbable::codegen::TestType, improbable::codegen::TestType>");
            Assert.That(unrealMapTypeReference.UnrealType == "UImprobableCodegenTestTypeToImprobableCodegenTestTypeMap*");

            Assert.That(unrealMapTypeReference.AssignUnderlyingValueToUnrealMemberVariable("TestField", "val") == "if (TestField == nullptr) { TestField = NewObject<UImprobableCodegenTestTypeToImprobableCodegenTestTypeMap>(this); } TestField->Init(val)");
            Assert.That(unrealMapTypeReference.ConvertUnderlyingValueToUnrealLocalVariable("val") == "NewObject<UImprobableCodegenTestTypeToImprobableCodegenTestTypeMap>()->Init(val)");
            Assert.That(unrealMapTypeReference.ConvertUnderlyingValueToUnrealMemberVariable("val") == "NewObject<UImprobableCodegenTestTypeToImprobableCodegenTestTypeMap>(this)->Init(val)");
            Assert.That(unrealMapTypeReference.ConvertUnrealValueToSnapshotValue("val") == "val->GetUnderlying()");
            Assert.That(unrealMapTypeReference.ConvertUnrealValueToUnderlyingValue("val") == "val->GetUnderlying()");
        }

        [Test]
        public static void unrealmaptypereference_is_initiated_as_expected_when_passed_a_map_type_reference_with_a_enums_as_contained_types()
        {
            var userEnumDefinition = GenerateEnumDefinition();

            var unrealEnumDetails = new UnrealEnumDetails(userEnumDefinition, "TestEnum", null);
            var containedKeyTypeReference = new UnrealEnumTypeReference(unrealEnumDetails);
            var containedValueTypeReference = new UnrealEnumTypeReference(unrealEnumDetails);
            var unrealMapTypeReference = new UnrealMapTypeReference(containedKeyTypeReference, containedValueTypeReference);

            Assert.That(unrealMapTypeReference.UnderlyingCapitalisedName == null);
            Assert.That(unrealMapTypeReference.RequiredIncludes.Count == 2);
            Assert.That(unrealMapTypeReference.RequiredIncludes.Contains("\"ImprobableCodegenTestEnumToImprobableCodegenTestEnumMap.h\""));
            Assert.That(unrealMapTypeReference.RequiredIncludes.Contains("\"TestEnum.h\""));
            Assert.That(unrealMapTypeReference.UnderlyingQualifiedName == "worker::Map<improbable::codegen::TestEnum, improbable::codegen::TestEnum>");
            Assert.That(unrealMapTypeReference.UnrealType == "UImprobableCodegenTestEnumToImprobableCodegenTestEnumMap*");

            Assert.That(unrealMapTypeReference.AssignUnderlyingValueToUnrealMemberVariable("TestField", "val") == "if (TestField == nullptr) { TestField = NewObject<UImprobableCodegenTestEnumToImprobableCodegenTestEnumMap>(this); } TestField->Init(val)");
            Assert.That(unrealMapTypeReference.ConvertUnderlyingValueToUnrealLocalVariable("val") == "NewObject<UImprobableCodegenTestEnumToImprobableCodegenTestEnumMap>()->Init(val)");
            Assert.That(unrealMapTypeReference.ConvertUnderlyingValueToUnrealMemberVariable("val") == "NewObject<UImprobableCodegenTestEnumToImprobableCodegenTestEnumMap>(this)->Init(val)");
            Assert.That(unrealMapTypeReference.ConvertUnrealValueToUnderlyingValue("val") == "val->GetUnderlying()");
        }

        private static TypeDefinitionRaw GenerateTypeDefinition()
        {
            var typeDefinitionRaw = new TypeDefinitionRaw();
            typeDefinitionRaw.name = "TestType";
            typeDefinitionRaw.qualifiedName = "improbable.codegen.TestType";
            typeDefinitionRaw.sourceReference = new SourceReferenceRaw() { line = "1", column = "1" };
            typeDefinitionRaw.optionSettings = null;
            typeDefinitionRaw.enumDefinitions = null;
            typeDefinitionRaw.typeDefinitions = null;
            typeDefinitionRaw.fieldDefinitions = null;

            return typeDefinitionRaw;
        }

        private static EnumDefinitionRaw GenerateEnumDefinition()
        {
            var enumDefinitionRaw = new EnumDefinitionRaw();
            enumDefinitionRaw.name = "TestEnum";
            enumDefinitionRaw.qualifiedName = "improbable.codegen.TestEnum";
            enumDefinitionRaw.sourceReference = new SourceReferenceRaw() { line = "1", column = "1" };
            enumDefinitionRaw.valueDefinitions = new EnumDefinitionRaw.ValueDefinitionRaw[]
            {
                new EnumDefinitionRaw.ValueDefinitionRaw
                {
                    name = "Test",
                    value = "1",
                    sourceReference = new SourceReferenceRaw() { line = "1", column = "1" }
                }
            };

            return enumDefinitionRaw;
        }
    }
}
