// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class BandLibrary : ModuleRules
{
    public BandLibrary(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;
        string DataPath = Path.Combine("$(PluginDir)", "Source", "ThirdParty", "BandLibrary", "Data");

        RuntimeDependencies.Add(Path.Combine(DataPath, "runtime_config.json"), StagedFileType.UFS);

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDelayLoadDLLs.Add("tensorflowlite_c.dll");
            RuntimeDependencies.Add(Path.Combine(DataPath, "Release", "tensorflowlite_c.dll"));
        }
		// TODO(dostos): not tested on iOS yet
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDelayLoadDLLs.Add("tensorflowlite_c.dylib");
            RuntimeDependencies.Add(Path.Combine(DataPath, "Release", "tensorflowlite_c.dylib"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
            string ExampleSoPath = Path.Combine(DataPath, "Release", "libtensorflowlite_c.so");
            PublicAdditionalLibraries.Add(ExampleSoPath);
            PublicDelayLoadDLLs.Add(ExampleSoPath);
            RuntimeDependencies.Add(ExampleSoPath);

            AdditionalPropertiesForReceipt.Add(new ReceiptProperty("AndroidPlugin", Path.Combine(PluginPath, "Band_arm64-v8a.xml")));

        }
    }
}
