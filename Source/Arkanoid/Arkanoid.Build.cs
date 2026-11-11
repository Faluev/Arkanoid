using UnrealBuildTool;

public class Arkanoid : ModuleRules
{
    public Arkanoid(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });
        PrivateDependencyModuleNames.AddRange(new[] { "EnhancedInput", "InputCore" });
    }
}
