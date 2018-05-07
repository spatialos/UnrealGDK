using System.Collections.Generic;
using System.IO;
using Improbable.CodeGeneration.Model;
using Improbable.CodeGeneration.Test;
using Improbable.CodeGeneration.Utils;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;
using NUnit.Framework;

namespace Improbable.Unreal.CodeGeneration.Test.SchemaProcessing.Unreal.TypeReferences
{
    public class UnrealCommandDetailsTest
    {
        [Test]
        public static void unrealcommanddetails_has_the_correct_capitalised_name_upon_creation()
        {
            var capitalisedName = "CapitalisedName";
            var qualifiedName = "improbable.codegen.Test";
            var capitalisedOwnerComponent = "OwnerComponent";
            var commandDefinition = GenerateCommandDefinition("Test",
                                                              new TypeReferenceRaw
                                                              {
                                                                  sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                                                                  builtInType = "double"
                                                              },
                                                              new TypeReferenceRaw
                                                              {
                                                                  sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                                                                  builtInType = "double"
                                                              });
            var packageDetails = GeneratePackageDetails();

            var requestTypeReference = new UnrealBuiltInTypeReference(commandDefinition.requestType);
            var responseTypeReference = new UnrealBuiltInTypeReference(commandDefinition.responseType);

            var commandDetails = new UnrealCommandDetails(commandDefinition, capitalisedName, qualifiedName, capitalisedOwnerComponent, requestTypeReference, responseTypeReference, packageDetails);

            Assert.That(commandDetails.CapitalisedName == capitalisedName, "the field CapitalisedName did not have the expected value upon creation");
        }

        [Test]
        public static void unrealcommanddetails_has_the_correct_underlying_command_definition_upon_creation()
        {
            var capitalisedName = "CapitalisedName";
            var qualifiedName = "improbable.codegen.Test";
            var capitalisedOwnerComponent = "OwnerComponent";
            var commandDefinition = GenerateCommandDefinition("Test",
                                                              new TypeReferenceRaw
                                                              {
                                                                  sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                                                                  builtInType = "double"
                                                              },
                                                              new TypeReferenceRaw
                                                              {
                                                                  sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                                                                  builtInType = "double"
                                                              });
            var packageDetails = GeneratePackageDetails();

            var requestTypeReference = new UnrealBuiltInTypeReference(commandDefinition.requestType);
            var responseTypeReference = new UnrealBuiltInTypeReference(commandDefinition.responseType);

            var commandDetails = new UnrealCommandDetails(commandDefinition, capitalisedName, qualifiedName, capitalisedOwnerComponent, requestTypeReference, responseTypeReference, packageDetails);

            Assert.That(commandDetails.UnderlyingCommandDefinition == commandDefinition, "the field UnderlyingCommandDefinition did not have the expected value upon creation");
        }

        [Test]
        public static void unrealcommanddetails_has_the_correct_underlying_capitalised_name_upon_creation()
        {
            var capitalisedName = "CapitalisedName";
            var qualifiedName = "improbable.codegen.Test";
            var capitalisedOwnerComponent = "OwnerComponent";
            var commandDefinition = GenerateCommandDefinition("Test",
                                                              new TypeReferenceRaw
                                                              {
                                                                  sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                                                                  builtInType = "double"
                                                              },
                                                              new TypeReferenceRaw
                                                              {
                                                                  sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                                                                  builtInType = "double"
                                                              });
            var packageDetails = GeneratePackageDetails();

            var requestTypeReference = new UnrealBuiltInTypeReference(commandDefinition.requestType);
            var responseTypeReference = new UnrealBuiltInTypeReference(commandDefinition.responseType);

            var commandDetails = new UnrealCommandDetails(commandDefinition, capitalisedName, qualifiedName, capitalisedOwnerComponent, requestTypeReference, responseTypeReference, packageDetails);

            Assert.That(commandDetails.UnderlyingCapitalisedName == Formatting.SnakeCaseToCapitalisedCamelCase(commandDefinition.name), "the field UnderlyingCapitalisedName did not have the expected value upon creation");
        }

        [Test]
        public static void unrealcommanddetails_has_the_correct_package_details_upon_creation()
        {
            var capitalisedName = "CapitalisedName";
            var qualifiedName = "improbable.codegen.Test";
            var capitalisedOwnerComponent = "OwnerComponent";
            var commandDefinition = GenerateCommandDefinition("Test",
                                                              new TypeReferenceRaw
                                                              {
                                                                  sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                                                                  builtInType = "double"
                                                              },
                                                              new TypeReferenceRaw
                                                              {
                                                                  sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                                                                  builtInType = "double"
                                                              });
            var packageDetails = GeneratePackageDetails();

            var requestTypeReference = new UnrealBuiltInTypeReference(commandDefinition.requestType);
            var responseTypeReference = new UnrealBuiltInTypeReference(commandDefinition.responseType);

            var commandDetails = new UnrealCommandDetails(commandDefinition, capitalisedName, qualifiedName, capitalisedOwnerComponent, requestTypeReference, responseTypeReference, packageDetails);

            Assert.That(commandDetails.UnderlyingPackageDetails == packageDetails, "the field UnderlyingPackageDetails did not have the expected value upon creation");
        }

        [Test]
        public static void unrealcommanddetails_has_the_correct_qualified_owner_name_upon_creation()
        {
            var capitalisedName = "CapitalisedName";
            var qualifiedOwnerName = "improbable.codegen.Test";
            var captialisedOwnerName = "improbable.codegen.OwnerComponent";
            var commandDefinition = GenerateCommandDefinition("Test",
                                                              new TypeReferenceRaw
                                                              {
                                                                  sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                                                                  builtInType = "double"
                                                              },
                                                              new TypeReferenceRaw
                                                              {
                                                                  sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                                                                  builtInType = "double"
                                                              });
            var packageDetails = GeneratePackageDetails();

            var requestTypeReference = new UnrealBuiltInTypeReference(commandDefinition.requestType);
            var responseTypeReference = new UnrealBuiltInTypeReference(commandDefinition.responseType);

            var commandDetails = new UnrealCommandDetails(commandDefinition, capitalisedName, qualifiedOwnerName, captialisedOwnerName, requestTypeReference, responseTypeReference, packageDetails);

            Assert.That(commandDetails.QualifiedOwnerName == qualifiedOwnerName, "the field QualifiedOwnerName did not have the expected value upon creation");
        }

        [Test]
        public static void unrealcommanddetails_has_the_correct_capitalised_owner_name_upon_creation()
        {
            var capitalisedName = "CapitalisedName";
            var qualifiedOwnerName = "improbable.codegen.Test";
            var captialisedOwnerName = "improbable.codegen.OwnerComponent";
            var commandDefinition = GenerateCommandDefinition("Test",
                                                              new TypeReferenceRaw
                                                              {
                                                                  sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                                                                  builtInType = "double"
                                                              },
                                                              new TypeReferenceRaw
                                                              {
                                                                  sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                                                                  builtInType = "double"
                                                              });
            var packageDetails = GeneratePackageDetails();

            var requestTypeReference = new UnrealBuiltInTypeReference(commandDefinition.requestType);
            var responseTypeReference = new UnrealBuiltInTypeReference(commandDefinition.responseType);

            var commandDetails = new UnrealCommandDetails(commandDefinition, capitalisedName, qualifiedOwnerName, captialisedOwnerName, requestTypeReference, responseTypeReference, packageDetails);

            Assert.That(commandDetails.CapitalisedOwnerName == captialisedOwnerName, "the field CapitalisedOwnerName did not have the expected value upon creation");
        }

        [Test]
        public static void unrealcommanddetails_has_the_correct_unreal_request_type_details_upon_creation()
        {
            var capitalisedName = "CapitalisedName";
            var qualifiedOwnerName = "improbable.codegen.Test";
            var captialisedOwnerName = "improbable.codegen.OwnerComponent";
            var commandDefinition = GenerateCommandDefinition("Test",
                                                              new TypeReferenceRaw
                                                              {
                                                                  sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                                                                  builtInType = "float"
                                                              },
                                                              new TypeReferenceRaw
                                                              {
                                                                  sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                                                                  builtInType = "float"
                                                              });
            var packageDetails = GeneratePackageDetails();

            var requestTypeReference = new UnrealBuiltInTypeReference(commandDefinition.requestType);
            var responseTypeReference = new UnrealBuiltInTypeReference(commandDefinition.responseType);

            var commandDetails = new UnrealCommandDetails(commandDefinition, capitalisedName, qualifiedOwnerName, captialisedOwnerName, requestTypeReference, responseTypeReference, packageDetails);

            Assert.That(commandDetails.UnrealRequestTypeDetails.UnderlyingCapitalisedName == requestTypeReference.UnderlyingCapitalisedName &&
                        commandDetails.UnrealRequestTypeDetails.UnderlyingQualifiedName == requestTypeReference.UnderlyingQualifiedName &&
                        commandDetails.UnrealRequestTypeDetails.UnrealType == requestTypeReference.UnrealType &&
                        commandDetails.UnrealRequestTypeDetails.RequiredIncludes == requestTypeReference.RequiredIncludes,
                        "the field UnrealRequestTypeDetails did not have the expected value upon creation");

            Assert.That(commandDetails.UnrealResponseTypeDetails.UnderlyingCapitalisedName == responseTypeReference.UnderlyingCapitalisedName &&
                        commandDetails.UnrealResponseTypeDetails.UnderlyingQualifiedName == responseTypeReference.UnderlyingQualifiedName &&
                        commandDetails.UnrealResponseTypeDetails.UnrealType == responseTypeReference.UnrealType &&
                        commandDetails.UnrealResponseTypeDetails.RequiredIncludes == responseTypeReference.RequiredIncludes,
                        "the field UnrealResponseTypeDetails did not have the expected value upon creation");
        }

        [Test]
        public static void unrealcommanddetails_has_the_correct_unreal_command_delegate_name_upon_creation()
        {
            var capitalisedName = "CapitalisedName";
            var qualifiedOwnerName = "improbable.codegen.Test";
            var captialisedOwnerName = "improbable.codegen.OwnerComponent";
            var commandDefinition = GenerateCommandDefinition("Test",
                                                              new TypeReferenceRaw
                                                              {
                                                                  sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                                                                  builtInType = null,
                                                                  userType = "DamageResponse"
                                                              },
                                                              new TypeReferenceRaw
                                                              {
                                                                  sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                                                                  builtInType = null,
                                                                  userType = "DamageRequest"
                                                              });
            var packageDetails = GeneratePackageDetails();

            var fieldDefinitionFloat = ModelTypeFactory.GenerateFieldDefinition("test", "1", new TypeReferenceRaw
            {
                sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                builtInType = "float"
            });

            var fieldDefinitionInt32 = ModelTypeFactory.GenerateFieldDefinition("test", "1", new TypeReferenceRaw
            {
                sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                builtInType = "int32"
            });

            var userTypeDefinition = ModelTypeFactory.GenerateTypeDefinition("TestType", "improbable.codegen.TestType", null, null, new FieldDefinitionRaw[]
            {
                fieldDefinitionFloat,
                fieldDefinitionInt32
            });

            var unrealFieldDetails = new List<UnrealFieldDetails>();
            unrealFieldDetails.Add(new UnrealFieldDetails(fieldDefinitionFloat, new UnrealBuiltInTypeReference(fieldDefinitionFloat.singularType)));
            unrealFieldDetails.Add(new UnrealFieldDetails(fieldDefinitionInt32, new UnrealBuiltInTypeReference(fieldDefinitionInt32.singularType)));

            var unrealTypeDetails = new UnrealTypeDetails(userTypeDefinition, "TestType", unrealFieldDetails, null);
            var requestTypeReference = new UnrealUserTypeReference(unrealTypeDetails);
            var responseTypeReference = new UnrealUserTypeReference(unrealTypeDetails);

            var commandDetails = new UnrealCommandDetails(commandDefinition, capitalisedName, qualifiedOwnerName, captialisedOwnerName, requestTypeReference, responseTypeReference, packageDetails);

            Assert.That(commandDetails.UnrealCommandDelegateName == string.Format("F{0}Command", capitalisedName), "the field UnrealCommandDelegateName did not have the expected value upon creation");
        }

        private static ComponentDefinitionRaw.CommandDefinitionRaw GenerateCommandDefinition(string name, TypeReferenceRaw requestType = null, TypeReferenceRaw responseType = null)
        {
            var commandDefinitionRaw = new ComponentDefinitionRaw.CommandDefinitionRaw();
            commandDefinitionRaw.name = name;
            commandDefinitionRaw.requestType = requestType;
            commandDefinitionRaw.responseType = responseType;
            commandDefinitionRaw.sourceReference = new SourceReferenceRaw() { line = "1", column = "1" };

            return commandDefinitionRaw;
        }

        private static UnrealPackageDetails GeneratePackageDetails()
        {
            return new UnrealPackageDetails(Path.Combine("improbable", Path.Combine("codegen", "Test.schema")), Path.Combine("improbable", Path.Combine("codegen", "Test.schema")), "improbable.codegen");
        }

        private static List<UnrealTypeDetails> GenerateUnrealTypeDetailsForResponderDependentTypes()
        {
            return new List<UnrealTypeDetails>
            {
                new UnrealTypeDetails(GenerateEmptyTypeDefinition("Request", "improbable.codegen.Request"), "Request", new List<UnrealFieldDetails>(), GeneratePackageDetails()),
                new UnrealTypeDetails(GenerateEmptyTypeDefinition("Response", "improbable.codegen.Response"), "Response", new List<UnrealFieldDetails>(), GeneratePackageDetails())
            };
        }

        private static TypeDefinitionRaw GenerateEmptyTypeDefinition(string name, string qualifiedName)
        {
            var typeDefinitionRaw = new TypeDefinitionRaw();
            typeDefinitionRaw.name = name;
            typeDefinitionRaw.qualifiedName = qualifiedName;
            typeDefinitionRaw.sourceReference = new SourceReferenceRaw();
            typeDefinitionRaw.enumDefinitions = new EnumDefinitionRaw[] { };
            typeDefinitionRaw.fieldDefinitions = new FieldDefinitionRaw[] { };
            typeDefinitionRaw.optionSettings = new OptionSettingRaw[] { };
            typeDefinitionRaw.typeDefinitions = new TypeDefinitionRaw[] { };

            return typeDefinitionRaw;
        }
    }
}
