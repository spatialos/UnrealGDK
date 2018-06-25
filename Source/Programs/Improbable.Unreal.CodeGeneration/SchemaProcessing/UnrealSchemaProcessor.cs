using System;
using System.Collections.Generic;
using System.Linq;
using Improbable.CodeGeneration.Model;
using Improbable.CodeGeneration.Utils;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;

namespace Improbable.Unreal.CodeGeneration.SchemaProcessing
{
    public class UnrealSchemaProcessor
    {
        public UnrealSchemaProcessor(ICollection<SchemaFileRaw> schemaFiles)
        {
            unrealPackageDetails = new Dictionary<string, UnrealPackageDetails>();
            resolvedEnumToCapitalisedNames = new Dictionary<EnumDefinitionRaw, string>();
            resolvedTypeToCapitalisedNames = new Dictionary<TypeDefinitionRaw, string>();
            resolvedCommandToCapitalisedNames = new Dictionary<ComponentDefinitionRaw.CommandDefinitionRaw, string>();
            resolvedComponentToCapitalisedNames = new Dictionary<ComponentDefinitionRaw, string>();

            ValidateNameClashes(schemaFiles);
            ResolveNames(schemaFiles);
            GeneratePackageDetails(schemaFiles);

            unrealEnumDetails = ProcessEnums(schemaFiles);
            unrealTypeDetails = ProcessTypes(schemaFiles);
            unrealCommandDetails = ProcessCommands(schemaFiles);
            unrealComponentDetails = ProcessComponents(schemaFiles);

            allPackageIncludes = new HashSet<string>();
            foreach (var unrealComponent in unrealComponentDetails)
            {
                allPackageIncludes.Add(unrealComponent.UnderlyingPackageDetails.Include);
            }
        }

        public readonly List<UnrealEnumDetails> unrealEnumDetails;
        public readonly List<UnrealTypeDetails> unrealTypeDetails;
        public readonly List<UnrealComponentDetails> unrealComponentDetails;
        public List<UnrealCommandDetails> unrealCommandDetails;
        public readonly HashSet<string> allPackageIncludes;

        private Dictionary<string, UnrealPackageDetails> unrealPackageDetails;

        private Dictionary<EnumDefinitionRaw, string> resolvedEnumToCapitalisedNames;
        private Dictionary<TypeDefinitionRaw, string> resolvedTypeToCapitalisedNames;
        private Dictionary<ComponentDefinitionRaw.CommandDefinitionRaw, string> resolvedCommandToCapitalisedNames;
        private Dictionary<ComponentDefinitionRaw, string> resolvedComponentToCapitalisedNames;

        private Dictionary<string, EnumDefinitionRaw> qualifiedNameToEnumDefinition;
        private Dictionary<string, TypeDefinitionRaw> qualifiedNameToTypeDefinition;

        public Dictionary<EnumDefinitionRaw, UnrealEnumDetails> enumDefinitionToUnrealEnum = new Dictionary<EnumDefinitionRaw, UnrealEnumDetails>();
        public readonly Dictionary<TypeDefinitionRaw, UnrealTypeDetails> typeDefinitionToUnrealType = new Dictionary<TypeDefinitionRaw, UnrealTypeDetails>();

        private void ValidateNameClashes(ICollection<SchemaFileRaw> schemaFiles)
        {
            var nonGeneratedSchemaFiles = schemaFiles.Select(schema => schema).Where(schema => !schema.package.Contains("improbable.unreal.generated"));

            var allComponentDefinitions = nonGeneratedSchemaFiles.SelectMany(entry => entry.componentDefinitions).ToList();
            var allCommandDefinitions = allComponentDefinitions.SelectMany(entry => entry.commandDefinitions).ToList();
            var allEnumDefinitions = nonGeneratedSchemaFiles.SelectMany(entry => entry.enumDefinitions).ToList();
            var allTypeDefinitions = nonGeneratedSchemaFiles.SelectMany(entry => entry.typeDefinitions).ToList();

            var duplicateNames = allComponentDefinitions.GroupBy(entry => entry.name)
                                                     .Where(g => g.Count() > 1)
                                                     .Select(entry => entry.Key)
                                                     .ToList();

            duplicateNames.AddRange(allEnumDefinitions.GroupBy(entry => entry.name)
                                                      .Where(g => g.Count() > 1)
                                                      .Select(entry => entry.Key)
                                                      .ToList());

            duplicateNames.AddRange(allTypeDefinitions.GroupBy(entry => entry.name)
                                                      .Where(g => g.Count() > 1)
                                                      .Select(entry => entry.Key)
                                                      .ToList());

            duplicateNames.AddRange(allCommandDefinitions.GroupBy(entry => entry.name)
                                                         .Where(g => g.Count() > 1)
                                                         .Select(entry => Formatting.SnakeCaseToCapitalisedCamelCase(entry.Key))
                                                         .ToList());

            //  Unreal doesn't support namespaces, therefore we can't use the same name in different schemas.
            if (duplicateNames.Count > 0)
            {
                throw new Exception(string.Format("You can't use the same name for multiple commands, components, types or enums:\n{0}", String.Join("\n", duplicateNames.ToArray())));
            }

            var duplicateComponentTypes = allComponentDefinitions.Select(entry => String.Format("{0}Component", entry.name)).ToList();
            duplicateComponentTypes.AddRange(allTypeDefinitions.Select(entry => entry.name).ToList());
            duplicateComponentTypes = duplicateComponentTypes.GroupBy(entry => entry).Where(g => g.Count() > 1).Select(entry => entry.Key).ToList();
            if (duplicateComponentTypes.Count > 0)
            {
                throw new Exception(string.Format("You can't use the same name for components and types:\n{0}", String.Join("\n", duplicateComponentTypes.ToArray())));
            }
        }

        private void ResolveNames(ICollection<SchemaFileRaw> schemaFiles)
        {
            var nonGeneratedSchemaFiles = schemaFiles.Select(schema => schema).Where(schema => !schema.package.Contains("improbable.unreal.generated"));

            var allComponentDefinitions = schemaFiles.SelectMany(entry => entry.componentDefinitions).ToList();
            var allCommandDefinitions = allComponentDefinitions.SelectMany(entry => entry.commandDefinitions).ToList();
            var allEnumDefinitions = schemaFiles.SelectMany(entry => entry.enumDefinitions).ToList();
            var allTypeDefinitions = schemaFiles.SelectMany(entry => entry.typeDefinitions).ToList();

            resolvedComponentToCapitalisedNames = allComponentDefinitions.ToDictionary(entry => entry, entry => entry.name);
            resolvedEnumToCapitalisedNames = allEnumDefinitions.ToDictionary(entry => entry, entry => entry.name);
            resolvedTypeToCapitalisedNames = allTypeDefinitions.ToDictionary(entry => entry, entry => entry.name);
            resolvedCommandToCapitalisedNames = allCommandDefinitions.ToDictionary(entry => entry, entry => Formatting.SnakeCaseToCapitalisedCamelCase(entry.name));
        }

        private void GeneratePackageDetails(ICollection<SchemaFileRaw> schemaFiles)
        {
            foreach (var schemaFile in schemaFiles)
            {
                unrealPackageDetails.Add(schemaFile.canonicalName, new UnrealPackageDetails(schemaFile.canonicalName, schemaFile.completePath, schemaFile.package));
            }
        }

        private List<UnrealEnumDetails> ProcessEnums(ICollection<SchemaFileRaw> schemaFiles)
        {
            var allEnumDefinitions = schemaFiles.SelectMany(schemaFile => schemaFile.enumDefinitions);
            qualifiedNameToEnumDefinition = allEnumDefinitions.ToDictionary(enumDefinition => enumDefinition.qualifiedName, enumDefinition => enumDefinition);

            var unrealEnums = new List<UnrealEnumDetails>();
            foreach (var schemaFile in schemaFiles)
            {
                var packageDetails = unrealPackageDetails[schemaFile.canonicalName];
                foreach (var enumDefinition in schemaFile.enumDefinitions)
                {
                    string capitalisedEnumName;
                    if (resolvedEnumToCapitalisedNames.TryGetValue(enumDefinition, out capitalisedEnumName))
                    {
                        var unrealEnum = new UnrealEnumDetails(enumDefinition, capitalisedEnumName, packageDetails);
                        unrealEnums.Add(unrealEnum);

                        enumDefinitionToUnrealEnum.Add(enumDefinition, unrealEnum);
                    }
                }
            }

            return unrealEnums;
        }

        private List<UnrealTypeDetails> ProcessTypes(ICollection<SchemaFileRaw> schemaFiles)
        {
            var allTypeDefinitions = schemaFiles.SelectMany(schemaFile => schemaFile.typeDefinitions);
            qualifiedNameToTypeDefinition = allTypeDefinitions.ToDictionary(typeDefinition => typeDefinition.qualifiedName, typeDefinition => typeDefinition);

            var unrealTypes = new List<UnrealTypeDetails>();
            foreach (var schemaFile in schemaFiles)
            {
                var packageDetails = unrealPackageDetails[schemaFile.canonicalName];

                foreach (var typeDefinition in schemaFile.typeDefinitions)
                {
                    var capitalisedTypeName = resolvedTypeToCapitalisedNames[typeDefinition];

                    var unrealType = new UnrealTypeDetails(typeDefinition, capitalisedTypeName, new List<UnrealFieldDetails>(), packageDetails);
                    unrealTypes.Add(unrealType);

                    typeDefinitionToUnrealType.Add(typeDefinition, unrealType);
                }
            }

            ProcessFieldDetails(schemaFiles);

            return unrealTypes;
        }

        private void ProcessFieldDetails(IEnumerable<SchemaFileRaw> schemaFiles)
        {
            foreach (var schemaFile in schemaFiles)
            {
                foreach (var typeDefinition in schemaFile.typeDefinitions)
                {
                    var fieldDetails = typeDefinition.fieldDefinitions.Select(fieldDefinition =>
                    {
                        var typeReference = GenerateUnrealTypeReference(fieldDefinition);
                        return new UnrealFieldDetails(fieldDefinition, typeReference);
                    });

                    typeDefinitionToUnrealType[typeDefinition].FieldDetailsList.AddRange(fieldDetails);
                }
            }
        }

        private List<UnrealComponentDetails> ProcessComponents(ICollection<SchemaFileRaw> schemaFiles)
        {
            var unrealComponents = new List<UnrealComponentDetails>();
            foreach (var schemaFile in schemaFiles)
            {
                var packageDetails = unrealPackageDetails[schemaFile.canonicalName];
                foreach (var componentDefinition in schemaFile.componentDefinitions)
                {
                    var capitalisedComponentName = resolvedComponentToCapitalisedNames[componentDefinition];

                    var commandTypeDefinition = qualifiedNameToTypeDefinition[componentDefinition.dataDefinition.TypeName];
                    var fieldDetails = typeDefinitionToUnrealType[commandTypeDefinition].FieldDetailsList;
                    var eventDetailsList = componentDefinition.eventDefinitions.Select(eventDefinition =>
                                                              {
                                                                  var typeReference = GenerateUnrealTypeReference(eventDefinition.type);
                                                                  return new UnrealEventDetails(eventDefinition, typeReference);
                                                              })
                                                              .ToList();

                    eventDetailsList.ForEach(individualEventDetails =>
                    {
                        var eventUserTypeReference =
                            individualEventDetails.EventTypeReference as UnrealUserTypeReference;
                        if (eventUserTypeReference != null)
                        {
                            eventUserTypeReference.TypeDetails.SetIsUsedForEvent();
                        }
                    });

                    // Extract the commands defined in the current component.
                    var commandDetails = unrealCommandDetails.Where(command => capitalisedComponentName == command.CapitalisedOwnerName).ToList();

                    var unrealComponent = new UnrealComponentDetails(componentDefinition, capitalisedComponentName, fieldDetails, eventDetailsList, commandDetails, packageDetails);

                    unrealComponents.Add(unrealComponent);
                }
            }

            return unrealComponents;
        }

        private List<UnrealCommandDetails> ProcessCommands(ICollection<SchemaFileRaw> schemaFiles)
        {
            var commandDetailsList = new List<UnrealCommandDetails>();
            foreach (var schemaFile in schemaFiles)
            {
                var packageDetails = unrealPackageDetails[schemaFile.canonicalName];
                if (packageDetails.IsAutoGenerated)
                {
                    continue;
                }

                foreach (var componentDefinition in schemaFile.componentDefinitions)
                {
                    var capitalisedComponentName = resolvedComponentToCapitalisedNames[componentDefinition];
                    foreach (var commandDefinition in componentDefinition.commandDefinitions)
                    {
                        var capitalisedCommandName = resolvedCommandToCapitalisedNames[commandDefinition];
                        var qualifiedComponentName = Formatting.QualifiedNameToCppQualifiedName(componentDefinition.qualifiedName);

                        var requestTypeReference = GenerateUnrealTypeReference(commandDefinition.requestType);
                        var responseTypeReference = GenerateUnrealTypeReference(commandDefinition.responseType);

                        var newCommand = new UnrealCommandDetails(commandDefinition, capitalisedCommandName, qualifiedComponentName, capitalisedComponentName, requestTypeReference, responseTypeReference, packageDetails);

                        if (!commandDetailsList.Exists(x => IsConflicting(x, newCommand)))
                        {
                            commandDetailsList.Add(newCommand);
                        }
                    }
                }
            }

            return commandDetailsList;
        }

        private bool IsConflicting(UnrealCommandDetails existingCommand, UnrealCommandDetails newCommand)
        {
            // We can not have commands with the same name and the same delegate type (request type + response type), even if they are in different components.
            // Ideally we could have commands with the same name and same delegate type (request type + response type) if they are in different components.
            if (existingCommand.CapitalisedName == newCommand.CapitalisedName && existingCommand.UnrealCommandDelegateType == newCommand.UnrealCommandDelegateType)
            {
                return true;
            }

            return false;
        }

        private IUnrealTypeReference GenerateUnrealTypeReference(FieldDefinitionRaw fieldDefinition)
        {
            IUnrealTypeReference unrealTypeReference;
            if (fieldDefinition.IsMap())
            {
                var containedKey = GenerateUnrealTypeReference(fieldDefinition.mapType.keyType);
                var containedValue = GenerateUnrealTypeReference(fieldDefinition.mapType.valueType);
                unrealTypeReference = new UnrealMapTypeReference(containedKey, containedValue);
            }
            else if (fieldDefinition.IsOption())
            {
                var containedType = GenerateUnrealTypeReference(fieldDefinition.optionType.valueType);
                unrealTypeReference = new UnrealOptionTypeReference(containedType);
            }
            else if (fieldDefinition.IsList())
            {
                var containedType = GenerateUnrealTypeReference(fieldDefinition.listType.valueType);
                unrealTypeReference = new UnrealListTypeReference(containedType);
            }
            else
            {
                unrealTypeReference = GenerateUnrealTypeReference(fieldDefinition.singularType);
            }

            return unrealTypeReference;
        }

        private IUnrealTypeReference GenerateUnrealTypeReference(TypeReferenceRaw typeReference)
        {
            IUnrealTypeReference unrealTypeReference;
            if (typeReference.IsBuiltInType)
            {
                unrealTypeReference = new UnrealBuiltInTypeReference(typeReference);
            }
            else if (qualifiedNameToEnumDefinition.ContainsKey(typeReference.TypeName))
            {
                var enumDefinition = qualifiedNameToEnumDefinition[typeReference.TypeName];
                var unrealEnumDetails = enumDefinitionToUnrealEnum[enumDefinition];
                unrealTypeReference = new UnrealEnumTypeReference(unrealEnumDetails);
            }
            else
            {
                var typeDefinition = qualifiedNameToTypeDefinition[typeReference.TypeName];
                var unrealTypeDetails = typeDefinitionToUnrealType[typeDefinition];
                unrealTypeReference = new UnrealUserTypeReference(unrealTypeDetails);
            }

            return unrealTypeReference;
        }
    }
}
