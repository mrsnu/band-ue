// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class BandLibrary : ModuleRules
{
	public BandLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		RuntimeDependencies.Add(new RuntimeDependency("$(ProjectDir)/Binaries/ThirdParty/BandLibrary/Data/runtime_config.json"));

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add("tensorflowlite_c.dll");

			// Ensure that the DLL is staged along with the executable
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/BandLibrary/x64/Release/tensorflowlite_c.dll");
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "Mac", "Release", "libExampleLibrary.dylib"));
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/BandLibrary/Mac/Release/libExampleLibrary.dylib");
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			string ExampleSoPath = Path.Combine("$(PluginDir)", "Binaries", "ThirdParty", "BandLibrary", "Linux", "x86_64-unknown-linux-gnu", "libExampleLibrary.so");
			PublicAdditionalLibraries.Add(ExampleSoPath);
			PublicDelayLoadDLLs.Add(ExampleSoPath);
			RuntimeDependencies.Add(ExampleSoPath);
		}
	}
}
