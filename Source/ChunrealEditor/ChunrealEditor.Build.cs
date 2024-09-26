// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ChunrealEditor : ModuleRules
{
    private bool bStrictIncludesCheck = true;

    public ChunrealEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseUnity = false;

        // This is to emulate engine installation and verify includes during development
        // Gives effect similar to BuildPlugin with -StrictIncludes
        if (bStrictIncludesCheck)
        {
            bUseUnity = false;
            PCHUsage = PCHUsageMode.NoPCHs;
            // Enable additional checks used for Engine modules
            bTreatAsEngineModule = true;
        }

        PublicIncludePaths.AddRange(
            new string[] {
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
                "AudioMixer",
                "AppFramework",
                "Chunreal",
                "AssetTools",
               "FileUtilities",
                 "AssetTools",
                "DetailCustomizations",
                 "EditorWidgets", 
                "EditorStyle",

                // ... add other public dependencies that you statically link with here ...
			}
            );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "UnrealEd",
                "Engine",
                "Slate",
                "SlateCore",
                "UMG",
                "UMGEditor",
                "Projects",
                "Blutility",
                "InputCore",
    

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