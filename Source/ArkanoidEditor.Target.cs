using UnrealBuildTool;

public class ArkanoidEditorTarget : TargetRules
{
    public ArkanoidEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("Arkanoid");
    }
}
