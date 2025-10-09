using UnrealBuildTool;

public class DatabaseEditor : ModuleRules
{
	public DatabaseEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
			}
			);


        PublicIncludePathModuleNames.AddRange(
            new string[]
            {
            }
        );
        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "DataTableEditor",
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "Database",
				"UnrealEd",
                "EditorStyle",
				"ContentBrowser",
            }
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
