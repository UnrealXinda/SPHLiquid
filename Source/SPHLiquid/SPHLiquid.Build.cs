// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class SPHLiquid : ModuleRules
{
	public SPHLiquid(ReadOnlyTargetRules Target) : base(Target)
	{
		string ModuleRootDirectory = Path.Combine(ModuleDirectory, "../..");
		string LiquidSimLib = "LiquidSimLib/lib/x64";
		string LiquidSimInclude = "LiquidSimLib/include";

		string CUDAPath = "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.2";
		string CUDAInclude = "include";
		string CUDALib = "lib/x64";

		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp17;

		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleRootDirectory, LiquidSimInclude),
				Path.Combine(CUDAPath, CUDAInclude),
			}
			);

		PublicAdditionalLibraries.AddRange(
			new string[] {
				Path.Combine(ModuleRootDirectory, LiquidSimLib, "liquid_sim.lib"),
				Path.Combine(CUDAPath, CUDALib, "cudart_static.lib"),
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
				"CoreUObject",
				"Engine",
				"RHI",
				"RenderCore",
				// ... add other public dependencies that you statically link with here ...
			}
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"Projects",
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
