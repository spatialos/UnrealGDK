using System;
using System.Collections.Generic;
using System.Linq;
using Improbable.CodeGeneration.Model;
using Improbable.CodeGeneration.Test;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;
using NUnit.Framework;

namespace Improbable.Unreal.CodeGeneration.SchemaProcessing.Test.Unreal
{
    public class UnrealSchemaProcessorTests
    {
        [Test]
        public static void process_types_components_throws_exception_when_clashing()
        {
            var schemaFileRaw1 = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.empty_component.json");
            var schemaFileRaw2 = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.component_type_clash.json");

            var schemaFilesRaw = new List<SchemaFileRaw> { schemaFileRaw1, schemaFileRaw2 };

            var exception = Assert.Throws<Exception>(() => { new UnrealSchemaProcessor(schemaFilesRaw); });
            Assert.AreEqual("You can't use the same name for components and types:\n" +
                            "EmptyComponentComponent",
                            exception.Message);
        }

        [Test]
        public static void process_types_creates_a_list_with_all_expected_types()
        {
            var schemaFileRaw = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.empty_component.json");

            var schemaFilesRaw = new List<SchemaFileRaw>() { schemaFileRaw };
            var schemaProcessor = new UnrealSchemaProcessor(schemaFilesRaw);

            Assert.That(schemaProcessor.unrealTypeDetails.Count == 1, "The schema processor did not contain the expected number of entries");
            Assert.That(schemaProcessor.unrealTypeDetails.First().UnderlyingTypeDefinition == schemaFilesRaw.First().typeDefinitions.First(), "The schema processor did not assign the type definition as expected");
        }

        [Test]
        public static void process_types_throws_exception_when_clashing()
        {
            var schemaFileRaw1 = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.empty_type.json");
            var schemaFileRaw2 = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.empty_type_clash.json");

            var schemaFilesRaw = new List<SchemaFileRaw> { schemaFileRaw1, schemaFileRaw2 };

            var exception = Assert.Throws<Exception>(() => { new UnrealSchemaProcessor(schemaFilesRaw); });

            Assert.AreEqual("You can't use the same name for multiple commands, components, types or enums:\n" +
                            "EmptyType",
                            exception.Message);
        }

        [Test]
        public static void process_commands_creates_a_list_with_all_expected_commands()
        {
            var schemaFileRaw = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.command.json");

            var schemaFilesRaw = new List<SchemaFileRaw>() { schemaFileRaw };
            var schemaProcessor = new UnrealSchemaProcessor(schemaFilesRaw);

            Assert.That(schemaProcessor.unrealCommandDetails.Count == 1, "The schema processor did not contain the expected number of entries");
            Assert.That(schemaProcessor.unrealCommandDetails.First().UnderlyingCommandDefinition == schemaFilesRaw.First().componentDefinitions.First().commandDefinitions.First(), "The schema processor did not assign the command definition as expected");
        }

        [Test]
        public static void process_commands_throws_exception_when_clashing()
        {
            var schemaFileRaw1 = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.command.json");
            var schemaFileRaw2 = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.command_clash.json");

            var schemaFilesRaw = new List<SchemaFileRaw>() { schemaFileRaw1, schemaFileRaw2 };

            var exception = Assert.Throws<Exception>(() => { new UnrealSchemaProcessor(schemaFilesRaw); });

            Assert.AreEqual("You can't use the same name for multiple commands, components, types or enums:\n" +
                            "ExampleCommand",
                            exception.Message);
        }

        [Test]
        public static void process_components_creates_a_list_with_all_expected_components()
        {
            var schemaFileRaw = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.empty_component.json");

            var schemaFilesRaw = new List<SchemaFileRaw> { schemaFileRaw };
            var schemaProcessor = new UnrealSchemaProcessor(schemaFilesRaw);

            Assert.That(schemaProcessor.unrealComponentDetails.Count == 1, "The schema processor did not contain the expected number of entries");
            Assert.That(schemaProcessor.unrealComponentDetails.First().UnderlyingComponentDefinition == schemaFilesRaw.First().componentDefinitions.First(), "The schema processor did not assign the component definition as expected");
        }

        [Test]
        public static void process_components_throws_exception_when_clashing()
        {
            var schemaFileRaw1 = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.empty_component.json");
            var schemaFileRaw2 = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.empty_component_clash.json");

            var schemaFilesRaw = new List<SchemaFileRaw> { schemaFileRaw1, schemaFileRaw2 };

            var exception = Assert.Throws<Exception>(() => { new UnrealSchemaProcessor(schemaFilesRaw); });

            Assert.AreEqual("You can't use the same name for multiple commands, components, types or enums:\n" +
                            "EmptyComponent\n" +
                            "EmptyComponentData",
                            exception.Message);
        }

        [Test]
        public static void process_enums_creates_a_list_with_all_expected_unreal_enums()
        {
            var schemaFileRaw = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.enum.json");

            var schemaFilesRaw = new List<SchemaFileRaw> { schemaFileRaw };
            var schemaProcessor = new UnrealSchemaProcessor(schemaFilesRaw);

            Assert.That(schemaProcessor.unrealEnumDetails.Count == 1, "The schema processor did not contain the expected number of entries");
            Assert.That(schemaProcessor.unrealEnumDetails.First().UnderlyingEnumDefinition == schemaFilesRaw.First().enumDefinitions.First(), "The schema processor did not assign the enum definition as expected");
        }

        [Test]
        public static void process_enums_throws_exception_when_clashing()
        {
            var schemaFileRaw1 = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.enum.json");
            var schemaFileRaw2 = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.enum_name_clash.json");

            var schemaFilesRaw = new List<SchemaFileRaw> { schemaFileRaw1, schemaFileRaw2 };

            var exception = Assert.Throws<Exception>(() => { new UnrealSchemaProcessor(schemaFilesRaw); });

            Assert.AreEqual("You can't use the same name for multiple commands, components, types or enums:\n" +
                            "ExampleColour",
                            exception.Message);
        }

        [Test]
        public static void process_types_allows_for_types_to_refer_to_types_declared_later_in_same_file()
        {
            var schemaFileRaw1 = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.type_reference_before_type.json");

            var schemaFilesRaw = new List<SchemaFileRaw>() { schemaFileRaw1 };
            var schemaProcessor = new UnrealSchemaProcessor(schemaFilesRaw);

            Assert.That(schemaProcessor.unrealTypeDetails.Count == 2, "The schema processor did not contain the expected number of entries");
            Assert.That(schemaProcessor.unrealTypeDetails[0].UnderlyingQualifiedName == "improbable::codegentests::TypeB", "The 'TypeB' type is not the first type to be observed");
            Assert.That(schemaProcessor.unrealTypeDetails[0].FieldDetailsList[0].TypeReference.UnderlyingQualifiedName == "improbable::codegentests::TypeA", "The 'TypeB' type does not have a reference to the 'TypeA' type");
            Assert.That(schemaProcessor.unrealTypeDetails[1].UnderlyingQualifiedName == "improbable::codegentests::TypeA", "The 'TypeA' type is not the second type to be observed");
        }


        [Test]
        public static void process_types_allows_for_types_to_refer_to_other_types_in_separate_files()
        {
            var typeAJson = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.file_order.file_order_a.json");
            var typeBJson = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.file_order.file_order_b.json");
            var typeCJson = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.file_order.file_order_c.json");

            var allJsonFiles = new List<SchemaFileRaw>() { typeAJson, typeBJson, typeCJson };
            var schemaProcessor = new UnrealSchemaProcessor(allJsonFiles);

            Assert.That(schemaProcessor.unrealTypeDetails.Count == 3, "The schema processor did not contain the expected number of entries");

            var typeATypeDetails = schemaProcessor.unrealTypeDetails[0];
            var typeBTypeDetails = schemaProcessor.unrealTypeDetails[1];
            var typeCTypeDetails = schemaProcessor.unrealTypeDetails[2];

            Assert.That(typeATypeDetails.UnderlyingQualifiedName == "improbable::codegentests::file_order::TypeA", "The 'TypeA' type is not the first type to be observed");
            Assert.That(typeBTypeDetails.UnderlyingQualifiedName == "improbable::codegentests::file_order::TypeB", "The 'TypeB' type is not the second type to be observed");
            Assert.That(typeCTypeDetails.UnderlyingQualifiedName == "improbable::codegentests::file_order::TypeC", "The 'TypeC' type is not the second type to be observed");

            var typeATypeBFieldReference = typeATypeDetails.FieldDetailsList[0];
            var typeCTypeBFieldReference = typeCTypeDetails.FieldDetailsList[0];

            Assert.That(typeATypeBFieldReference.TypeReference.UnderlyingQualifiedName == typeBTypeDetails.UnderlyingQualifiedName, "The 'TypeA' type does not have a reference to the 'TypeB' type");
            Assert.That(typeCTypeBFieldReference.TypeReference.UnderlyingQualifiedName == typeBTypeDetails.UnderlyingQualifiedName, "The 'TypeC' type does not have a reference to the 'TypeB' type");

            Assert.That(typeCTypeBFieldReference.TypeReference.DefaultInitialisationString == "improbable::codegentests::file_order::TypeB(false, false, false)", "The default initialisation string of TypeB property within TypeC is incorrect");
            Assert.That(typeATypeBFieldReference.TypeReference.DefaultInitialisationString == typeCTypeBFieldReference.TypeReference.DefaultInitialisationString, "The default initialisation string of TypeB property within TypeA is different than the default initialisation string of TypeB property within TypeC");
        }

        /**
         *  component BenchmarkingClient {
         *  id = 30001;
         *  command Timestamp ping(Timestamp);
         *  command Timestamp ping_time2(Timestamp);
         *  command Timestamp ping2(Void);
         *  command Timestamp ping3(Void);
         * } 
         * component BenchmarkingServer {
         *  id = 30002;
         *  command Timestamp verify_update(Timestamp);
         *  command Timestamp ping(Timestamp);
         * }
         */
        [Test]
        public static void process_multi_commands_can_have_same_delegate_type()
        {
            var commandJson = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.commands_have_same_delegate.json");

            var clientCmdNames = commandJson.componentDefinitions.ToList().Find(comp => comp.name == "BenchmarkingClient").commandDefinitions.Select(c => c.name).ToList();
            var serverCmdNames = commandJson.componentDefinitions.ToList().Find(comp => comp.name == "BenchmarkingServer").commandDefinitions.Select(c => c.name).ToList();
            clientCmdNames.Sort();
            serverCmdNames.Sort();
            var clientRawCmds = string.Join(", ", clientCmdNames.ToArray());
            var serverRawCmds = string.Join(", ", serverCmdNames.ToArray());
            Assert.That(clientRawCmds == "ping, ping_time2, ping2, ping3", string.Format("expect `ping, ping_time2, ping2, ping3` but got {0}", clientRawCmds));
            Assert.That(serverRawCmds == "ping4, verify_update", string.Format("expect `ping4, verify_update` but got {0}", serverRawCmds));

            UnrealSchemaProcessor unrealProcessor = new UnrealSchemaProcessor(new List<SchemaFileRaw>() { commandJson });
            var components = unrealProcessor.unrealComponentDetails;
            Assert.That(components.Count == 2, "Test schema should contain 2 components");

            var clientComponent = components.Find(c => c.CapitalisedName == "BenchmarkingClient");
            var serverComponent = components.Find(c => c.CapitalisedName == "BenchmarkingServer");
            Assert.That(clientComponent != null, "`BenchmarkingClient` should exist");
            Assert.That(serverComponent != null, "`BenchmarkingServer` should exist");

            var pingClientCommand = clientComponent.CommandDetailsList.Find(cmd => cmd.CapitalisedName == "Ping");
            var pingTime2Command = clientComponent.CommandDetailsList.Find(cmd => cmd.CapitalisedName == "PingTime2");
            var ping2Command = clientComponent.CommandDetailsList.Find(cmd => cmd.CapitalisedName == "Ping2");
            var ping3Command = clientComponent.CommandDetailsList.Find(cmd => cmd.CapitalisedName == "Ping3");
            var verifyCommand = serverComponent.CommandDetailsList.Find(cmd => cmd.CapitalisedName == "VerifyUpdate");
            var pingServerCommand = serverComponent.CommandDetailsList.Find(cmd => cmd.CapitalisedName == "Ping");

            Assert.That(pingClientCommand != null, "`BenchmarkingClient` should contain `Ping` command");
            Assert.That(pingTime2Command != null, "`BenchmarkingClient` should contain `PingTime2` command");
            Assert.That(ping2Command != null, "`BenchmarkingClient` should contain `Ping2` command");
            Assert.That(ping3Command != null, "`BenchmarkingClient` should contain `Ping3` command");
            Assert.That(verifyCommand != null, "`BenchmarkingServer` should contain `VerifyUpdate` command");
            // `Ping` command in `BenchmarkingServer` can not be generated due to the constraint we added in UnrealSchemaProcessor to accommodate the limitation in our API implementation.
            Assert.That(pingServerCommand == null, "`BenchmarkingServer` should not contain `Ping` command");

            Assert.That(pingClientCommand.UnrealCommandDelegateType == verifyCommand.UnrealCommandDelegateType, "Expect `Ping` in `BenchmarkingClient` has the same delegate as `VerifyUpdate` in `BenchmarkingServer` does");
            Assert.That(pingClientCommand.UnrealCommandDelegateType == pingTime2Command.UnrealCommandDelegateType, "Expect `Ping` in `BenchmarkingClient` has the same delegate as `PingTime2` does");
            Assert.That(ping2Command.UnrealCommandDelegateType == ping3Command.UnrealCommandDelegateType, "Expect `Ping2` has the same delegate as `Ping3` does");

            // Once we remove the constraint for command codegen, the following assertion should be valid.
            //Assert.That(pingClientCommand.UnrealCommandDelegateType == pingServerCommand.UnrealCommandDelegateType, "Expect `Ping` in `BenchmarkingClient` has the same delegate as `Ping` in `BenchmarkingServer` does");
        }

        [Test]
        public static void data_types_can_include_other_types_independent_of_order()
        {
            var commandJson = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.nested_type_order.json");

            var unrealProcessor = new UnrealSchemaProcessor(new List<SchemaFileRaw> { commandJson });

            var myComponentDetails = unrealProcessor.unrealComponentDetails.First();

            Assert.AreEqual("TestComponent", myComponentDetails.CapitalisedName, "The first component's name does not match an expected value.");
            Assert.AreEqual("TestComponentCustomData", myComponentDetails.CapitalisedDataName, "The first component's data type name does not match an expected value.");

            var firstFieldDetails = myComponentDetails.FieldDetailsList.First();

            Assert.AreEqual("USomeOtherType", firstFieldDetails.TypeReference.UClassName, "The field does not refer to the correct type");

            Assert.AreEqual("improbable::codegentests::nestedtypeorder::SomeOtherType(improbable::codegentests::nestedtypeorder::SomeType(worker::EntityId(0), 0))",
                            firstFieldDetails.TypeReference.DefaultInitialisationString,
                            "Type with nested type has wrong initialisation string");
        }

        [Test]
        public static void cyclic_dependencies_will_produce_a_helpful_error()
        {
            var commandJson =
                TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.nested_type_order.json");

            var someTypeDefinition = commandJson.typeDefinitions.First(definition => definition.name == "SomeType");

            var newFieldDefinitions = new List<FieldDefinitionRaw>(someTypeDefinition.fieldDefinitions);

            /* This inserts "SomeOtherType this_should_be_cyclic = 3;" into "SomeType".

               This means:
                 TestComponentCustomData
                   - SomeOtherType   TestComponentCustomData.my_member_for_component           // 1
                   - SomeType        SomeOtherType.my_member_within_some_other_type            // 2
                   - SomeOtherType   SomeType.this_should_be_cyclic                            // 3
                   - SomeType        SomeOtherType.my_member_within_some_other_type"           // 2 << cyclic dependency detected! Already generating default initialisation string for 2
            */
            {
                newFieldDefinitions.Add(new FieldDefinitionRaw
                {
                    name = "this_should_be_cyclic",
                    number = "3",
                    singularType = new TypeReferenceRaw
                    {
                        userType = "improbable.codegentests.nestedtypeorder.SomeOtherType"
                    },
                });

                someTypeDefinition.fieldDefinitions = newFieldDefinitions.ToArray();
            }

            var unrealProcessor = new UnrealSchemaProcessor(new List<SchemaFileRaw> { commandJson });

            var firstComponentDetails = unrealProcessor.unrealComponentDetails.First();
            var firstFieldDetails = firstComponentDetails.FieldDetailsList.First();

            string initialisationString = null;

            var exception = Assert.Throws<UnrealUserTypeReference.CyclicTypeReferenceException>(() => { initialisationString = firstFieldDetails.TypeReference.DefaultInitialisationString; });

            Assert.IsNull(initialisationString, "The initialisation string should not have been set in error cases.");

            Assert.AreEqual("Cyclic type reference detected while generating default initialisation string.\n" +
                            "Introduce an 'option', 'list', or 'map' field to break the chain.\n" +
                            "Generation stack:\n" +
                            " - SomeType SomeOtherType.my_member_within_some_other_type\n" +
                            " - SomeOtherType SomeType.this_should_be_cyclic\n" +
                            " - SomeType SomeOtherType.my_member_within_some_other_type",
                            exception.Message);
        }
    }
}
