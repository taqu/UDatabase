using System.IO;
using UnrealBuildTool;

public class Database : ModuleRules
{
	public Database(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		CStandard = CStandardVersion.Latest;
        CppStandard = CppStandardVersion.Cpp20;

		string ThirdPartyPath = Path.Combine(ModuleDirectory, "ThirdParty");
        PublicIncludePaths.AddRange(
			new string[] {
                Path.Combine(ThirdPartyPath, "Include")
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
			}
			);

		string ThirdPartyLibPath = Path.Combine(ThirdPartyPath, "Lib", "x64");
        PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibPath, "libcrypto.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibPath, "libssl.lib"));

		RuntimeDependencies.Add(Path.Combine(ThirdPartyLibPath, "libcrypto-3-x64.dll"));
        RuntimeDependencies.Add(Path.Combine(ThirdPartyLibPath, "libssl-3-x64.dll"));

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
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
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
        AppendStringToPublicDefinition("SQLITE_HAS_CODEC", "1");
        AppendStringToPublicDefinition("SQLITE_TEMP_STORE", "2");
        AppendStringToPublicDefinition("SQLITE_EXTRA_INIT", "sqlcipher_extra_init");
        AppendStringToPublicDefinition("SQLITE_EXTRA_SHUTDOWN", "sqlcipher_extra_shutdown");

        AppendStringToPublicDefinition("SQLITE_OS_WIN", "1");
        AppendStringToPublicDefinition("SQLITE_OS_UNIX", "0");
		AppendStringToPublicDefinition("SQLITE_OS_KV", "0");
		AppendStringToPublicDefinition("SQLITE_OS_OTHER", "0");

        AppendStringToPublicDefinition("SQLITE_ENABLE_HIDDEN_COLUMNS", "0");

        AppendStringToPublicDefinition("HAVE_STDINT_H", "1");
        AppendStringToPublicDefinition("HAVE_ISNAN", "1");
		AppendStringToPublicDefinition("SQLITE_HAVE_ISNAN", "1");
        AppendStringToPublicDefinition("HAVE_LOCALTIME_R", "0");
        AppendStringToPublicDefinition("HAVE_LOCALTIME_S", "1");
        AppendStringToPublicDefinition("HAVE_MALLOC_H", "1");
        AppendStringToPublicDefinition("HAVE_STRCHRNUL", "0");
        AppendStringToPublicDefinition("SQLITE_WIN32_FILEMAPPING_API", "1");
        AppendStringToPublicDefinition("SQLITE_WIN32_USE_UUID", "1");
        AppendStringToPublicDefinition("SQLITE_ENABLE_STAT4", "0");
        AppendStringToPublicDefinition("HAVE_MALLOC_USABLE_SIZE", "0");
        AppendStringToPublicDefinition("SQLITE_ALLOW_ROWID_IN_VIEW", "0");
        AppendStringToPublicDefinition("SQLITE_DEFAULT_CKPTFULLFSYNC", "1");
    }
}
