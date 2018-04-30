using System.Collections.Generic;

namespace Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences
{
    public sealed class UnrealOptionTypeReference : UnrealTypeReference
    {
        public readonly IUnrealTypeReference UnderlyingValueType;

        public UnrealOptionTypeReference(IUnrealTypeReference containedTypeReference)
        {
            ReferenceType = ReferenceType.Option;
            UnderlyingValueType = containedTypeReference;

            var className = string.Format("{0}Option", containedTypeReference.UnderlyingCapitalisedName);
            UnrealType = string.Format("U{0}*", className);
            UnderlyingQualifiedName = containedTypeReference.UnderlyingQualifiedName;
            UnderlyingCapitalisedName = containedTypeReference.UnderlyingCapitalisedName;
            ConvertUnderlyingValueToUnrealMemberVariable = (cppOption) =>
            {
                // Set owning object as new uobject's outer.
                return string.Format("NewObject<U{0}>(this)->Init({1})", className, cppOption);
            };
            AssignUnderlyingValueToUnrealMemberVariable = (capitalizedName, cppOption) =>
            {
                return string.Format(@"if ({0} == nullptr) {{ {0} = NewObject<U{1}>(this); }} {0}->Init({2})",
                                     capitalizedName,
                                     className,
                                     cppOption);
            };
            CheckInequality = (capitalizedName, compName) => { return string.Format("{0} && (*{0} != {1})", capitalizedName, compName); };
            ConvertUnderlyingValueToUnrealLocalVariable = (cppOption) =>
            {
                // Set static instance package as the uobject's outer.
                return string.Format("NewObject<U{0}>()->Init({1})", className, cppOption);
            };
            ConvertUnrealValueToSnapshotValue = (VarName) => { return ConvertUnrealValueToUnderlyingValue(VarName); };
            ConvertUnrealValueToUnderlyingValue = (unrealOption) => { return string.Format("{0}->GetUnderlying()", unrealOption); };
            RequiredIncludes = new List<string>();
            RequiredIncludes.Add(string.Format("\"{0}.h\"", className));
            RequiredIncludes.AddRange(containedTypeReference.RequiredIncludes);

            DefaultInitialisationString = string.Format("worker::Option<{0}>()", UnderlyingQualifiedName);
            ArgumentName = UnrealType;
            SnapshotType = string.Format("worker::Option<{0}>", UnderlyingQualifiedName);
            UClassName = string.Format("U{0}", className);
            DefaultValue = "nullptr";
        }
    }
}
