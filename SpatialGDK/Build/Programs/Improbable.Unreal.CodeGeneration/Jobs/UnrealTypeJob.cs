using System.Collections.Generic;
using Improbable.CodeGeneration.FileHandling;
using Improbable.CodeGeneration.Jobs;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;

namespace Improbable.Unreal.CodeGeneration.Jobs
{
    public class UnrealTypeJob : CodegenJob
    {
        public UnrealTypeJob(UnrealTypeDetails unrealType, HashSet<string> generatedMaps, HashSet<string> generatedOptions, HashSet<string> generatedLists, string outputDirectory, IFileSystem fileSystem)
            : base(outputDirectory, fileSystem)
        {
            InputFiles = new List<string>() { unrealType.UnderlyingPackageDetails.SourceSchema };
            OutputFiles = new List<string>();

            var headerGenerator = new UnrealTypeHeaderGenerator(unrealType);
            var typeHeaderFileName = unrealType.CapitalisedName + headerSuffix;
            OutputFiles.Add(typeHeaderFileName);
            Content.Add(typeHeaderFileName, headerGenerator.TransformText());

            var implementationGenerator = new UnrealTypeImplementationGenerator(unrealType);
            var typeImplFileName = unrealType.CapitalisedName + cppSuffix;
            OutputFiles.Add(typeImplFileName);
            Content.Add(typeImplFileName, implementationGenerator.TransformText());

            foreach (var fieldDefinition in unrealType.FieldDetailsList)
            {
                if (fieldDefinition.IsMap())
                {
                    var mapTypeReference = fieldDefinition.TypeReference as UnrealMapTypeReference;
                    var filename = string.Format("{0}To{1}Map", mapTypeReference.KeyType.UnderlyingCapitalisedName, mapTypeReference.ValueType.UnderlyingCapitalisedName);
                    if (generatedMaps.Contains(filename))
                    {
                        continue;
                    }

                    generatedMaps.Add(filename);

                    var mapHeaderGenerator = new UnrealMapHeaderGenerator(mapTypeReference);
                    var mapHeaderFileName = filename + headerSuffix;
                    OutputFiles.Add(mapHeaderFileName);
                    Content.Add(mapHeaderFileName, mapHeaderGenerator.TransformText());

                    var mapImplementationGenerator = new UnrealMapImplementationGenerator(mapTypeReference);
                    var mapImplFileName = filename + cppSuffix;
                    OutputFiles.Add(mapImplFileName);
                    Content.Add(mapImplFileName, mapImplementationGenerator.TransformText());
                }

                if (fieldDefinition.IsOption())
                {
                    var optionTypeReference = fieldDefinition.TypeReference as UnrealOptionTypeReference;

                    var filename = string.Format("{0}Option", fieldDefinition.TypeReference.UnderlyingCapitalisedName);
                    if (generatedOptions.Contains(filename))
                    {
                        continue;
                    }

                    generatedOptions.Add(filename);
                    var optionHeaderGenerator = new UnrealOptionHeaderGenerator(optionTypeReference);
                    var optionHeaderFileName = filename + headerSuffix;
                    OutputFiles.Add(optionHeaderFileName);
                    Content.Add(optionHeaderFileName, optionHeaderGenerator.TransformText());

                    var optionImplementationGenerator = new UnrealOptionImplementationGenerator(optionTypeReference);
                    var optionImplFileName = filename + cppSuffix;
                    OutputFiles.Add(optionImplFileName);
                    Content.Add(optionImplFileName, optionImplementationGenerator.TransformText());
                }

                if (fieldDefinition.IsList())
                {
                    var listTypeReference = fieldDefinition.TypeReference as UnrealListTypeReference;

                    var filename = string.Format("{0}List", fieldDefinition.TypeReference.UnderlyingCapitalisedName);
                    if (generatedLists.Contains(filename))
                    {
                        continue;
                    }

                    generatedLists.Add(filename);
                    var listHeaderGenerator = new UnrealListHeaderGenerator(listTypeReference);
                    var listHeaderFileName = filename + headerSuffix;
                    OutputFiles.Add(listHeaderFileName);
                    Content.Add(listHeaderFileName, listHeaderGenerator.TransformText());

                    var listImplementationGenerator = new UnrealListImplementationGenerator(listTypeReference);
                    var listImplFileName = filename + cppSuffix;
                    OutputFiles.Add(listImplFileName);
                    Content.Add(listImplFileName, listImplementationGenerator.TransformText());
                }
            }
        }

        protected override void RunImpl()
        {
            //Do nothing here for now
        }

        private const string headerSuffix = ".h";
        private const string cppSuffix = ".cpp";
    }
}
