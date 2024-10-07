Support and Feedback (for this fork)
https://discord.gg/EWFKWTz3

Status:
1. Fixed file loading on windows (doesn't work on main repo).
2. Reading/writing global variables via ChuckID is not possible when using the 'new style' processor
3. work in progress - real time chuck code editor, monitors the chuck files in the WorkingDirectory and automatically recompiles the chuck vm on changes, with a nice in-game widget

https://github.com/user-attachments/assets/88edb1d8-1f97-4d69-8c2e-0e19875bf209


4. ChucK Processors can now be applied as Source and Submix effect in preset chains.  

https://github.com/user-attachments/assets/accddb23-fffd-4d21-80f7-6d6ad3640467

5. Real time parameter getters and setters within metasounds - 

https://github.com/user-attachments/assets/780c46c0-8e04-432e-936f-ac7ac1afeca2

6. Execute and Listen to Chuck events from BP or metasounds -

https://github.com/user-attachments/assets/d0448a87-2391-456c-b007-1e897b20202f



# Purpose of fork -
This fork encapsulates the chunreal repo as a plugin and makes some changes to the way the ChucK classes interact with Unreal.

The UChuckProcessor is a UObject that can be created as an asset in the editor where ChucK code can be input (with a work in progress syntax highlighter), this object has an audio object proxy and a registered meta sound data type and can be passed directly to the new Chuck Midi Renderer metasound node, the ChucK is compiled when the object proxy is created and thus should be able to play once assigned to the metasound node.

This repo is a work in progress and some aspects are still not working really well and might require redesign.

The end result should be reusable chuck assets that can be used inside metasounds with no BP setup and without needing to trigger compilation on the audio thread, making it simple to use ChucK components as instruments or effects in metasounds or in unDAW: https://github.com/Amir-BK/unDAW



## New workflow!

### Automatic monitoring of chuck source files
The ChunrealEditor module monitors the working directory (inside the plugin folder) for .ck files and will automatically create chuck processor assets for these sources, changes to the source files are tracked and will trigger recompilation of chuck vms using these proxies, this is still a work in progress but there are many advantages to keeping the chucks as source files outside unreal, primarily being able to add them to source control and easily editing them from text editors outside unreal.

At the moment (a matter of days if not hours) it is the 'Chuck Project Editor' that monitors the directory rather than the module itself, the plan is not to have any dependencies on Editor Only functionality so that these chuck sources can also potentially be exposed and modified in packaged games at runtime.

### Manual creation (to be redesigned)

Instead of assigning the code via blueprints there's a new 'Chuck Processor' asset that can be created via the content browser - 
![image](https://github.com/user-attachments/assets/27a6adf3-393b-4b7d-89fb-42ea7423ed2d)

ChucK code can be added to this asset: 
![image](https://github.com/user-attachments/assets/fe5e23cb-bd60-43b1-bf35-8ff2229b6858)

Once this asset has been created and saved it can be used in a Metasound using the new Chuck Midi Renderer Node, despite its name this node doesn't _have_ to be used with Midi, although at the moment it is not being assigned a ChucK global id and thus can only receive new params via midi, this will be fixed soon.
![image](https://github.com/user-attachments/assets/b0fbaca1-fe4a-43dd-b1bf-847059910656)

https://github.com/user-attachments/assets/0d10b089-70f9-415d-a0bf-332c5cbbcfcc

Currently the midi renderer is not polyphonic, working on that, the midi renderer node executes the 'noteEvent' and 'noteFreq' params on the chuck instance whenever it reads a note from the harmonix stream, you can see how to set up a simple instrument in the example midi renderer chuck processor provided in the project.

With these changes it is possible create re-usable chuck processor assets and use them in metasounds (as midi instruments or effects) without needing to interact with the metasound from blueprints.

All previously existing Chunreal nodes and methods still work.

The changes also make it possible to use Chunk instruments and effects easily with unDAW and the metasound builder system - check out the unDAW repo - https://github.com/Amir-BK/unDAW

https://github.com/user-attachments/assets/588dbf3c-8bdb-4e63-aa8b-cb0f1601bfd2

## Loading samples and files from disk
The 'working directory' is currently set to "chunreal/WorkingDirectory" but when you try to load a sample with a relative unqualified path it will still search for it in the engine binary folder, you can make chucK look for samples in the working dir with the following syntax:

```
me.dir() + "kick_01.wav" => kick.read;
```

the working directory path can be changed in code, it is assigned to the ChucK when it is instantiated (so in the midi renderer node code), it can be exposed as a config variable and assigned per project, I'll eventually get to that, for now the WorkingDirectory inside the plugin seems usable.



# Chunreal
## ChucK - Strongly-timed Music Programming Language - in Unreal Engine 5
**_Chunreal_** is a plugin for [Unreal Engine 5](https://www.unrealengine.com/)  that allows users to compile [ChucK](https://github.com/ccrma/chuck) code at runtime. Multiple ChuckMain nodes can be chained in a MetaSound graph using stereo input & output to perform modular synthesis style digital signal processing. 

Developed by [Eito Murakami](https://ccrma.stanford.edu/~eitom) and [Ge Wang](https://ccrma.stanford.edu/~ge) with the help of Max Jardetzky at [CCRMA | Stanford University](https://ccrma.stanford.edu/).

## Wiki Tutorial!
The Wiki page contains a step-by-step tutorial that demonstrates how to set up the Chunreal plugin in your Unreal Engine project.

https://github.com/ccrma/chunreal/wiki

## Template Project

This repo contains all the content from the original Chunreal repository only encapsulated inside the plugin package, all the example content can be found inside the plugin's content folder, there's also a midi file to test the new node with.

### Try Example Scenes!
Try opening the following example levels and playing in the editor viewport!
- **Chunreal_SetGlobals_ExampleLevel**
- **Chunreal_ChainedDSP_ExampleLevel**
- **Chunreal_1Source_ExampleLevel**
- **Chunreal_ManySources_ExampleLevel**
- **Chunreal_Physics_ExampleLevel**
- **Chunreal_Mic_ExampleLevel**
- **Chunreal_GlobalEvent_ExampleLevel**
- **Chunreal_PitchChange_ExampleLevel**


## How To Use Chunreal
### Setting Up ChuckMain Node
**Chunreal_Simple_MetaSound** asset contains an example of ChuckMain node implementation in a MetaSound source. 

Input
- **Run Code**: A trigger input for compiling ChucK code.
- **Code**: A string input for ChucK code to be compiled.
- **ID**: A string input for assigning a unique ID to a ChuckMain node. This is used to get and set global variables for a specific ChucK instance.
- **Audio Input Left**: Audio input left channel. Can be accessed by adc.left inside a ChucK code.
- **Audio Input Right**: Audio input right channel. Can be accessed by adc.right inside a ChucK code.
- **Volume Multiplier**: A float input for applying a volume multiplier to both output channels.

Output
- **Audio Output Left**: Audio output left channel. Can be accessed by dac.left inside a ChucK code.
- **Audio Output Right**: Audio output right channel. Can be accessed by dac.right inside a ChucK code.
<img width="683" alt="image" src="https://github.com/ccrma/chunreal/assets/75334216/ca0cd851-843a-4435-8557-efe61fa23a55">

### Set Parameters From A Blueprint Actor
Create a Blueprint actor, attach an audio component, and assign your MetaSound source that contains a ChuckMain node(s) as the sound parameter.
Optionally, enable _Allow Spatialization_ and apply _Attenuation Settings_. We prepared an example **Binaural_SA** sound attenuation asset.

**_Chunreal_Simple_BP_** asset provides a template.

<img width="512" alt="image" src="https://github.com/ccrma/chunreal/assets/75334216/1afd5fe6-a1a5-436a-88a9-baf57edc03ee">

A ChucK code can be written as a string and can be passed to the Audio Component that uses the MetaSound source as follows:

<img width="1004" alt="image" src="https://github.com/ccrma/chunreal/assets/75334216/5df1d56b-2b3a-4b3c-ae17-2868ba5b839a">

Alternatively, we prepared a **ReadFile** Blueprint function for loading a _.ck_ file using an absolute path.

<img width="1025" alt="image" src="https://github.com/ccrma/chunreal/assets/75334216/5bd443ba-c303-4b9b-9bee-174fe22a52f0">

### Getting & Setting ChucK Global Variables Using ID
A specific ChucK instance can be accessed using its assigned unique ID. Global _event_, _int_, _float_, and _string_ variables can be accessed using the following Blueprint functions.
To learn how to use each function, open each Blueprint Actor in **_ExampleBlueprints_** folder. 

<img width="421" alt="image" src="https://github.com/ccrma/chunreal/assets/75334216/8f3ea549-a607-4592-b129-b96db6421ede">

### Connecting Multiple ChuckMain Nodes
Multiple ChuckMain nodes can be chained in a MetaSound source and can interact with other existing MetaSound nodes!

**Chunreal_ChainedDSPExample_MetaSound** and **Chunreal_ChainedDSPExample_BP** provide a template.

<img width="1117" alt="image" src="https://github.com/ccrma/chunreal/assets/75334216/34271b38-185e-4d43-91b8-65a8b8da8c63">


## ChucK Community
Join us!! [ChucK Community Discord](https://discord.gg/ENr3nurrx8) | [ChucK-users Mailing list](https://lists.cs.princeton.edu/mailman/listinfo/chuck-users)
