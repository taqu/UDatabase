using System.IO;
using UnrealBuildTool;

public class UDatabase : ModuleRules
{
	public UDatabase(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		CStandard = CStandardVersion.Latest;
        CppStandard = CppStandardVersion.Latest;

        PrivateDefinitions.Add("UDATABASE_PASSWORD=PASSWORD");

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "SQLiteCore",
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
			}
			);

        PublicIncludePathModuleNames.AddRange(
            new string[] {
                "SQLiteCore",
            }
        );

        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
    }
}
