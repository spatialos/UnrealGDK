// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;
using System.Collections.Generic;

public class SdkEditorTarget : TargetRules
{
    public SdkEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        ExtraModuleNames.Add("Sdk");

        bForceCompileDevelopmentAutomationTests = true;
    }
}
