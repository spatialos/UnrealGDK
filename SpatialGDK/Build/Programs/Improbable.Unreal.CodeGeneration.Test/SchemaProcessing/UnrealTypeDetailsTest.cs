using System.Collections.Generic;
using System.IO;
using System.Linq;
using Improbable.CodeGeneration.Model;
using Improbable.CodeGeneration.Utils;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;
using NUnit.Framework;

namespace Improbable.Unreal.CodeGeneration.Test.SchemaProcessing.Unreal
{
    public class UnrealTypeDetailsTest
    {
        [Test]
        public static void unrealtypedetails_has_the_correct_capitalised_name_upon_creation()
        {
            var capitalisedName = "CapitalisedName";
            var typeDefinition = GenerateTypeDefinition("TestType", "improbable.codegen.TestType");
            var packageDetails = GeneratePackageDetails();
            var typeDetails = new UnrealTypeDetails(typeDefinition, capitalisedName, new List<UnrealFieldDetails>(), packageDetails);

            Assert.That(typeDetails.CapitalisedName == capitalisedName, "the field CapitalisedName did not have the expected value upon creation");
        }

        [Test]
        public static void unrealtypedetails_has_the_correct_underlying_type_definition_upon_creation()
        {
            var capitalisedName = "CapitalisedName";
            var typeDefinition = GenerateTypeDefinition("TestType", "improbable.codegen.TestType");
            var packageDetails = GeneratePackageDetails();
            var typeDetails = new UnrealTypeDetails(typeDefinition, capitalisedName, new List<UnrealFieldDetails>(), packageDetails);

            Assert.That(typeDetails.UnderlyingTypeDefinition == typeDefinition, "the field UnderlyingTypeDefinition did not have the expected value upon creation");
        }

        [Test]
        public static void unrealtypedetails_has_the_correct_package_details_upon_creation()
        {
            var capitalisedName = "CapitalisedName";
            var typeDefinition = GenerateTypeDefinition("TestType", "improbable.codegen.TestType");
            var packageDetails = GeneratePackageDetails();
            var typeDetails = new UnrealTypeDetails(typeDefinition, capitalisedName, new List<UnrealFieldDetails>(), packageDetails);

            Assert.That(typeDetails.UnderlyingPackageDetails == packageDetails, "the field UnderlyingPackageDetails did not have the expected value upon creation");
        }

        [Test]
        public static void unrealtypedetails_has_the_correct_underlying_capitalised_name_upon_creation()
        {
            var capitalisedName = "CapitalisedName";
            var typeDefinition = GenerateTypeDefinition("TestType", "improbable.codegen.TestType");
            var packageDetails = GeneratePackageDetails();
            var typeDetails = new UnrealTypeDetails(typeDefinition, capitalisedName, new List<UnrealFieldDetails>(), packageDetails);

            Assert.That(typeDetails.UnderlyingCapitalisedName == "TestType", "the field UnderlyingCapitalisedName did not have the expected value upon creation");
        }

        [Test]
        public static void unrealtypedetails_has_the_correct_underlying_qualified_name_upon_creation()
        {
            var capitalisedName = "CapitalisedName";
            var typeDefinition = GenerateTypeDefinition("TestType", "improbable.codegen.TestType");
            var packageDetails = GeneratePackageDetails();
            var typeDetails = new UnrealTypeDetails(typeDefinition, capitalisedName, new List<UnrealFieldDetails>(), packageDetails);

            Assert.That(typeDetails.UnderlyingQualifiedName == Formatting.QualifiedNameToCppQualifiedName(typeDefinition.qualifiedName), "the field UnderlyingQualifiedName did not have the expected value upon creation");
        }

        [Test]
        public static void unrealtypedetails_has_the_correct_capitalised_qualified_name_upon_creation()
        {
            var capitalisedName = "CapitalisedName";
            var typeDefinition = GenerateTypeDefinition("TestType", "improbable.codegen.TestType");
            var packageDetails = GeneratePackageDetails();
            var typeDetails = new UnrealTypeDetails(typeDefinition, capitalisedName, new List<UnrealFieldDetails>(), packageDetails);

            Assert.That(typeDetails.CapitalisedQualifiedName == Formatting.QualifiedNameToCapitalisedCamelCase(typeDefinition.qualifiedName), "the field CapitalisedQualifiedName did not have the expected value upon creation");
        }

        [Test]
        public static void unrealtypedetails_has_the_correct_field_details_list_name_upon_creation()
        {
            var capitalisedName = "CapitalisedName";
            var fieldDefinitionRaw = GenerateFieldDefinition("test", "1", new TypeReferenceRaw
            {
                sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                builtInType = "float"
            });
            var typeDefinition = GenerateTypeDefinition("TestType", "improbable.codegen.TestType", null, null, new FieldDefinitionRaw[] { fieldDefinitionRaw });
            var packageDetails = GeneratePackageDetails();

            var fieldDetails = new List<UnrealFieldDetails>();
            fieldDetails.Add(new UnrealFieldDetails(fieldDefinitionRaw, new UnrealBuiltInTypeReference(fieldDefinitionRaw.singularType)));

            var typeDetails = new UnrealTypeDetails(typeDefinition, capitalisedName, fieldDetails, packageDetails);

            Assert.That(typeDetails.FieldDetailsList.Count() == 1, "The wrong number of field details were created during the creation of an unreal type details object");

            Assert.That(typeDetails.FieldDetailsList.FirstOrDefault((fieldDetail) =>
            {
                return fieldDetail.CapitalisedName == "Test" &&
                       fieldDetail.LowercaseName == "test" &&
                       fieldDetail.TypeReference.UnrealType == "float";
            }) != null, "The generated field detail for the unreal type detail did not contain the correct data");
        }

        private static UnrealPackageDetails GeneratePackageDetails()
        {
            return new UnrealPackageDetails(TestSchemaPath, TestSchemaPath, "improbable.codegen");
        }

        private static TypeDefinitionRaw GenerateTypeDefinition(string name, string qualifiedName, EnumDefinitionRaw[] enumDefinitions = null, TypeDefinitionRaw[] typeDefinitions = null, FieldDefinitionRaw[] fieldDefinitions = null)
        {
            var typeDefinitionRaw = new TypeDefinitionRaw();
            typeDefinitionRaw.sourceReference = new SourceReferenceRaw { line = "1", column = "1" };
            typeDefinitionRaw.name = name;
            typeDefinitionRaw.qualifiedName = qualifiedName;
            typeDefinitionRaw.optionSettings = new OptionSettingRaw[] { };
            typeDefinitionRaw.enumDefinitions = enumDefinitions ?? new EnumDefinitionRaw[] { };
            typeDefinitionRaw.typeDefinitions = typeDefinitions ?? new TypeDefinitionRaw[] { };
            typeDefinitionRaw.fieldDefinitions = fieldDefinitions ?? new FieldDefinitionRaw[] { };

            return typeDefinitionRaw;
        }

        private static FieldDefinitionRaw GenerateFieldDefinition(string name, string number, TypeReferenceRaw singularType = null, FieldDefinitionRaw.OptionTypeRaw optionType = null, FieldDefinitionRaw.ListTypeRaw listType = null, FieldDefinitionRaw.MapTypeRaw mapType = null)
        {
            var fieldDefinitionRaw = new FieldDefinitionRaw();
            fieldDefinitionRaw.sourceReference = new SourceReferenceRaw() { line = "1", column = "1" };
            fieldDefinitionRaw.name = name;
            fieldDefinitionRaw.number = number;
            fieldDefinitionRaw.singularType = singularType;
            fieldDefinitionRaw.optionType = optionType;
            fieldDefinitionRaw.listType = listType;
            fieldDefinitionRaw.mapType = mapType;

            return fieldDefinitionRaw;
        }

        private static UnrealEnumDetails GenerateUnrealEnumDetails(string name, string qualifiedName)
        {
            var enumDefinitionRaw = new EnumDefinitionRaw();
            enumDefinitionRaw.name = name;
            enumDefinitionRaw.qualifiedName = qualifiedName;
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

            return new UnrealEnumDetails(enumDefinitionRaw, "Test", GeneratePackageDetails());
        }

        private static readonly string TestSchemaPath = Path.Combine("improbable", Path.Combine("codegen", "Test.schema"));
    }
}
