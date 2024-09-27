# Purpose of fork -
This fork encapsulates the chunreal repo as a plugin and makes some changes to the way the ChucK classes interact with Unreal.

The UChuckInstance is a UObject that can be crated as an asset in the editor where ChucK code can be input (with a work in progress syntax highlighter), this object has an audio object proxy and a registered meta sound data type and can be passed directly to the new Chuck Midi Renderer metasound node, the ChucK is compiled when the object proxy is created and thus should be able to play once assigned to the metasound node.

This repo is a work in progress and some aspects are still not working really well and might require redesign, for instance compiling the same UChuckInstance asset twice will lead to collision in the ChucK module as both will share the same ChuckID.

The end result should be reusable chuck assets that can be used inside metasounds with no BP setup and without needing to trigger compilation on the audio thread, making it simple to use ChucK components as instruments or effects in metasounds or in unDAW: https://github.com/Amir-BK/unDAW


## New workflow!

Instead of assigning the code via blueprints there's a new 'Chuck Processor' asset that can be created via the content browser - 
![image](https://github.com/user-attachments/assets/27a6adf3-393b-4b7d-89fb-42ea7423ed2d)

ChucK code can be added to this asset: 
![image](https://github.com/user-attachments/assets/fe5e23cb-bd60-43b1-bf35-8ff2229b6858)

Once this asset has been created and saved it can be used in a Metasound using the new Chuck Midi Renderer Node, despite its name this node doesn't _have_ to be used with Midi, although at the moment it is not being assigned a ChucK global id and thus can only receive new params via midi, this will be fixed soon.
![image](https://github.com/user-attachments/assets/b0fbaca1-fe4a-43dd-b1bf-847059910656)

https://github.com/user-attachments/assets/0d10b089-70f9-415d-a0bf-332c5cbbcfcc

Currently the midi renderer is not polyphonic, working on that, the midi renderer node executes the 'noteEvent' and 'noteFreq' params on the chuck instance whenever it reads a note from the harmonix stream, you can see how to set up a simple instrument in the example midi renderer chuck processor provided in the project.

With these changes it is possible create re-usable chuck processor assets and use them in metasounds (as midi instruments or effects) without needing to interact with the metasound from blueprints.

https://github.com/user-attachments/assets/3cbf0253-2bab-4e67-ba18-3aee5be4ad10


All previously existing Chunreal nodes and methods still work.


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
