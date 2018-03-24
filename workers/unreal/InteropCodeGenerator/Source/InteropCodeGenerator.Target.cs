
using UnrealBuildTool;
using System.Collections.Generic;

public class InteropCodeGeneratorTarget : TargetRules
{
	public InteropCodeGeneratorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		ExtraModuleNames.Add("InteropCodeGenerator");
	}
	public override void SetupGlobalEnvironment(
 		TargetInfo Target,
 		ref LinkEnvironmentConfiguration OutLinkEnvironmentConfiguration,
 		ref CPPEnvironmentConfiguration OutCPPEnvironmentConfiguration
 		)
 	{
 		bBuildEditor = false;
 		bBuildWithEditorOnlyData = false;
 	}
}
