using Improbable.CodeGeneration.Model;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;
using NUnit.Framework;

namespace Improbable.Unreal.CodeGeneration.Test.SchemaProcessing.Unreal.TypeReferences
{
    public class UnrealEnumTypeReferenceTest
    {
        [Test]
        public static void unrealenumtypereference_is_initiated_as_expected_when_passed_an_enum_type_reference()
        {
            var enumDefinition = GenerateEnumDefinition();

            var unrealEnumDetails = new UnrealEnumDetails(enumDefinition, "TestEnum", null);

            var unrealEnumTypeReference = new UnrealEnumTypeReference(unrealEnumDetails);

            Assert.That(unrealEnumTypeReference.UnderlyingCapitalisedName == "ImprobableCodegenTestEnum");
            Assert.That(unrealEnumTypeReference.RequiredIncludes.Count == 1);
            Assert.That(unrealEnumTypeReference.RequiredIncludes.Contains("\"TestEnum.h\""));
            Assert.That(unrealEnumTypeReference.UnderlyingQualifiedName == "improbable::codegen::TestEnum");
            Assert.That(unrealEnumTypeReference.UnrealType == "ETestEnum");

            Assert.That(unrealEnumTypeReference.AssignUnderlyingValueToUnrealMemberVariable("TestField", "val") == "TestField = static_cast<ETestEnum>(val)");
            Assert.That(unrealEnumTypeReference.ConvertUnderlyingValueToUnrealLocalVariable("val") == "static_cast<ETestEnum>(val)");
            Assert.That(unrealEnumTypeReference.ConvertUnderlyingValueToUnrealMemberVariable("val") == "static_cast<ETestEnum>(val)");
            Assert.That(unrealEnumTypeReference.ConvertUnrealValueToSnapshotValue("val") == "val");
            Assert.That(unrealEnumTypeReference.ConvertUnrealValueToUnderlyingValue("1") == "static_cast<improbable::codegen::TestEnum>(1)");
            Assert.That(unrealEnumTypeReference.DefaultInitialisationString == "static_cast<improbable::codegen::TestEnum>(0)");
            Assert.That(unrealEnumTypeReference.SnapshotType == "ETestEnum");
        }

        private static EnumDefinitionRaw GenerateEnumDefinition()
        {
            var enumDefinitionRaw = new EnumDefinitionRaw();
            enumDefinitionRaw.name = "TestEnum";
            enumDefinitionRaw.qualifiedName = "improbable.codegen.TestEnum";
            enumDefinitionRaw.sourceReference = new SourceReferenceRaw() { line = "1", column = "1" };
            enumDefinitionRaw.valueDefinitions = new EnumDefinitionRaw.ValueDefinitionRaw[]
            {
                new EnumDefinitionRaw.ValueDefinitionRaw()
                {
                    name = "RED",
                    value = "1"
                },
                new EnumDefinitionRaw.ValueDefinitionRaw()
                {
                    name = "GREEN",
                    value = "1"
                }
            };

            return enumDefinitionRaw;
        }
    }
}
