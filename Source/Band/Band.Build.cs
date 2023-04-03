// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Band : ModuleRules
{
	public Band(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// TODO(dostos): change `Data` dir to better name/structure
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "Data/Release/band_c.dll"));
			PublicDelayLoadDLLs.Add("band_c.dll");
		}
		// TODO(dostos): not tested on iOS yet
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "Data/Release/libtensorflowlite_c.dylib"));
			PublicDelayLoadDLLs.Add("libtensorflowlite_c.dylib");
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			string ExampleSoPath = Path.Combine(ModuleDirectory, "Data/Release/libband_c.so");
			PublicAdditionalLibraries.Add(ExampleSoPath);
			RuntimeDependencies.Add(ExampleSoPath);
			PublicDelayLoadDLLs.Add("libband_c.so");

			AdditionalPropertiesForReceipt.Add(new ReceiptProperty("AndroidPlugin",
				Path.Combine(ModuleDirectory, "Band.xml")));
		}

		PublicIncludePaths.AddRange(
			new string[]
			{
				Path.Combine(ModuleDirectory, "Library"),
				Path.Combine(ModuleDirectory, "ThirdParty")
				// ... add public include paths required here ...
			}
		);


		PrivateIncludePaths.AddRange(
			new string[]
			{
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
				"RHI",
				"UMG",
				"TraceLog",
				"JsonUtilities",
				"AndroidCamera"
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