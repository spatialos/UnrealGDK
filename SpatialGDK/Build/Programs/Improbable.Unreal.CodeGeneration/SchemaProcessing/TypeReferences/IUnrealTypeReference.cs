using System;
using System.Collections.Generic;

namespace Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences
{
    public enum ReferenceType
    {
        BuiltIn,
        Enum,
        List,
        Map,
        Option,
        UserType
    };

    public interface IUnrealTypeReference
    {
        ReferenceType ReferenceType { get; }
        string UnrealType { get; }
        Func<string, string> ConvertUnderlyingValueToUnrealMemberVariable { get; }
        Func<string, string, string> AssignUnderlyingValueToUnrealMemberVariable { get; }
        Func<string, string, string> CheckInequality { get; }
        Func<string, string> ConvertUnderlyingValueToUnrealLocalVariable { get; }
        Func<string, string> ConvertUnrealValueToSnapshotValue { get; }
        Func<string, string> ConvertUnrealValueToUnderlyingValue { get; }
        string UnderlyingQualifiedName { get; }
        string UnderlyingCapitalisedName { get; }
        List<string> RequiredIncludes { get; }
        string DefaultInitialisationString { get; }
        string ArgumentName { get; }
        string SnapshotType { get; }
        string UClassName { get; }
        string CapitalisedName { get; }
        string DefaultValue { get; }
    }
}
