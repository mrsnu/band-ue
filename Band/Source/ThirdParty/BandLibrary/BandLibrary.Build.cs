// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class BandLibrary : ModuleRules
{
	public BandLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/BandLibrary/Data/runtime_config.json", StagedFileType.UFS);

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicDelayLoadDLLs.Add("tensorflowlite_c.dll");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/BandLibrary/Data/Release/tensorflowlite_c.dll");
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDelayLoadDLLs.Add("libtensorflowlite_c.dylib");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/BandLibrary/Data/Release/libtensorflowlite_c.dylib");
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			string ExampleSoPath = Path.Combine("$(PluginDir)", "Source", "ThirdParty", "BandLibrary", "Data", "Release", "libtensorflowlite_c.so");
			PublicAdditionalLibraries.Add(ExampleSoPath);
			PublicDelayLoadDLLs.Add(ExampleSoPath);
			RuntimeDependencies.Add(ExampleSoPath);
		}
	}
}
