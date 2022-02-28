// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Band : ModuleRules
{
	public Band(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "Data/runtime_config.json"), StagedFileType.UFS);

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "Data/Release/tensorflowlite_c.dll"));
			PublicDelayLoadDLLs.Add("tensorflowlite_c.dll");
		}
		// TODO(dostos): not tested on iOS yet
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "Data/Release/libtensorflowlite_c.dylib"));
			PublicDelayLoadDLLs.Add("libtensorflowlite_c.dylib");
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			string ExampleSoPath = Path.Combine(ModuleDirectory, "Data/Release/libtensorflowlite_c.so");
			PublicAdditionalLibraries.Add(ExampleSoPath);
			RuntimeDependencies.Add(ExampleSoPath);
			PublicDelayLoadDLLs.Add("libtensorflowlite_c.so");

			AdditionalPropertiesForReceipt.Add(new ReceiptProperty("AndroidPlugin", Path.Combine(ModuleDirectory, "Band.xml")));
		}

		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "Library")
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Projects"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
