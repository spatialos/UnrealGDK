using System;
using System.Collections.Generic;
using System.Linq;
using Improbable.CodeGeneration.Utils;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences
{
    public sealed class UnrealUserTypeReference : UnrealTypeReference
    {
        /// <summary>
        ///     If a field refers to a user-defined type, there is a chance for cyclic dependency to happen; this exception
        ///     prevents that.
        /// </summary>
        /// <remarks>
        ///     e.g.:
        ///     TypeA has a field: TypeB my_type_b = 1;
        ///     TypeB has a field: TypeA my_type_a = 1;
        ///     Uses a "debug stack" in order to be able to explain what is going wrong to the user.
        /// </remarks>
        public class CyclicTypeReferenceException : Exception
        {
            public CyclicTypeReferenceException(IEnumerable<string> generationDebugStack)
                : base(CreateCyclicReferenceMessage(generationDebugStack)) { }

            private static string CreateCyclicReferenceMessage(IEnumerable<string> generationDebugStack)
            {
                return "Cyclic type reference detected while generating default initialisation string.\n" +
                       "Introduce an 'option', 'list', or 'map' field to break the chain.\n" +
                       "Generation stack:\n" +
                       string.Join("\n", generationDebugStack
                                         .Reverse()
                                         .Select(stackItem => string.Format(" - {0}", stackItem))
                                         .ToArray());
            }
        }

        // Used to catch cyclic dependencies
        private bool generatingDefaultInitialisationString = false;

        // This stack is used to print cyclic dependency problems while generating initialisation strings
        // See CyclicTypeReferenceException
        private static readonly Stack<string> GenerationDebugStack = new Stack<string>();

        private string defaultInitialisationString = null;

        /// <summary>
        ///     Lazily generates a default initialisation string.
        /// </summary>
        /// <returns>
        ///     A string with this format: MyTypeName({initialize first field}, {initialize second field}, ... all other
        ///     fields)
        /// </returns>
        /// <exception cref="CyclicTypeReferenceException">
        ///     If any of the fields of this type end up referring back to this type,
        ///     that would result in an infinitely long initialisation string.
        ///     e.g.:
        ///     TypeA(TypeB(TypeA(TypeB(TypeA( ...
        ///     This exception prevents that.
        /// </exception>
        public override string DefaultInitialisationString
        {
            get
            {
                if (defaultInitialisationString != null)
                {
                    return defaultInitialisationString;
                }

                if (generatingDefaultInitialisationString)
                {
                    throw new CyclicTypeReferenceException(GenerationDebugStack);
                }

                generatingDefaultInitialisationString = true;

                try
                {
                    var initialisationStringsForFields = TypeDetails.FieldDetailsList
                                                                    .Select(GetInitialisationStringForField).ToArray();

                    defaultInitialisationString = string.Format("{0}({1})", UnderlyingQualifiedName,
                                                                string.Join(", ", initialisationStringsForFields));

                    return defaultInitialisationString;
                }
                finally
                {
                    generatingDefaultInitialisationString = false;
                }
            }
        }

        private string GetInitialisationStringForField(UnrealFieldDetails fieldDefinition)
        {
            var fieldTypeReference = fieldDefinition.TypeReference;

            string fieldInitialisationString;

            var fieldUserTypeReference = fieldTypeReference as UnrealUserTypeReference;

            if (fieldUserTypeReference != null)
            {
                GenerationDebugStack.Push(string.Format("{0} {1}.{2}", fieldUserTypeReference.CapitalisedName, CapitalisedName, fieldDefinition.LowercaseName));

                try
                {
                    fieldInitialisationString = fieldUserTypeReference.DefaultInitialisationString;
                }
                finally
                {
                    // Let's make sure that the debug stack is clean even after a cyclic dependency error has been thrown.
                    GenerationDebugStack.Pop();
                }
            }
            else
            {
                fieldInitialisationString = fieldTypeReference.DefaultInitialisationString;
            }

            return fieldInitialisationString;
        }

        public UnrealUserTypeReference(UnrealTypeDetails unrealType)
        {
            ReferenceType = ReferenceType.UserType;
            UnderlyingQualifiedName = unrealType.UnderlyingQualifiedName;
            UnderlyingCapitalisedName = Formatting.CppQualifiedNameToCapitalisedCamelCase(UnderlyingQualifiedName);
            UnrealType = string.Format("U{0}*", unrealType.CapitalisedName);
            CapitalisedName = string.Format("{0}", unrealType.CapitalisedName);
            RequiredIncludes = new List<string>();
            RequiredIncludes.Add(string.Format("\"{0}.h\"", unrealType.CapitalisedName));
            ConvertUnderlyingValueToUnrealMemberVariable = (cppValue) =>
            {
                // Set owning object as new uobject's outer.
                return string.Format("NewObject<U{0}>(this)->Init({1})", unrealType.CapitalisedName, cppValue);
            };
            AssignUnderlyingValueToUnrealMemberVariable = (capitalizedName, cppValue) =>
            {
                return string.Format(@"if ({0} == nullptr) {{ {0} = NewObject<U{1}>(this); }} {0}->Init({2})",
                                     capitalizedName,
                                     unrealType.CapitalisedName,
                                     cppValue);
            };
            CheckInequality = (capitalizedName, compName) => { return string.Format("{0} && ({0}->GetUnderlying() != {1})", capitalizedName, compName); };
            ConvertUnderlyingValueToUnrealLocalVariable = (cppValue) =>
            {
                // Set static instance package as the uobject's outer.
                return string.Format("NewObject<U{0}>()->Init({1})", unrealType.CapitalisedName, cppValue);
            };
            ConvertUnrealValueToSnapshotValue = (VarName) => { return string.Format("{0}->GetUnderlying()", VarName); };
            ConvertUnrealValueToUnderlyingValue = (unrealValue) => { return string.Format("{0}->GetUnderlying()", unrealValue); };

            ArgumentName = UnrealType;
            SnapshotType = unrealType.UnderlyingQualifiedName;
            UClassName = string.Format("U{0}", unrealType.CapitalisedName);
            TypeDetails = unrealType;
            DefaultValue = "nullptr";
        }

        public UnrealTypeDetails TypeDetails { get; set; }
    }
}
