using UnrealBuildTool;

public class ArkanoidTarget : TargetRules
{
    public ArkanoidTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("Arkanoid");
    }
}
