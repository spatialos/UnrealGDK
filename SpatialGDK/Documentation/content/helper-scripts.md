
# Helper scripts

These scripts are located under `Plugins\UnrealGDK\SpatialGDK\Build\Scripts\` directory of your game.

| Helper script | Parameters | Description |
| --- | --- | --- |
| `BuildWorker.bat` | `<target> <platform> <configuration> <YourGame>.uproject [--skip-codegen]` | Run this script from the command line to [build your game’s Unreal server-workers and client-workers]({{urlRoot}}/content/glossary#workers) ready for uploading as a SpatialOS [cloud deployment]({{urlRoot}}/content/glossary#deployment) or set them up for testing in a local deployment in the Unreal Editor on your development machine. <br/><br/> Some parameters also [cook](https://docs.unrealengine.com/en-US/Engine/Deployment/Cooking) your workers to Unreal format and zip them up. (A SpatialOS cloud deployment requires built workers in zipped files.) <br/><br/> (In the example and list below, `<YourGame>` is the name of your Unreal project.) </br></br>**Example**</br> `BuildWorker.bat <YourGame>Editor Win64 Development <YourGame>.uproject` </br></br> **Flags** </br> **`<target>`**</br> This is the output file the built workers are stored in. They are [Unreal’s build target](https://docs.unrealengine.com/en-us/Programming/BuildTools/UnrealBuildTool/TargetFiles) files. The target can be: </br> * `<YourGame>Editor`: Set up server-workers for a local deployment for testing in the Unreal Editor on your development machine. This option does not cook or zip server-workers as this is not needed for local deployments. </br> * `<YourGame>Server`: Build, cook and zip server-workers ready for upload to the SpatialOS cloud as a cloud deployment. </br> * `<YourGame>`: Cooks a stand-alone version of the game to test it as a game client. </br></br> If you specify other Unreal  `<target>`, this script passes all arguments to the UE script, `\Engine\Build\BatchFiles\Build.bat`with no cooking or zipping. </br></br> **`<platform>`**</br> Specify the platform your built server-worker or game client to runs on. This is:</br> *  `linux` for your server-workers </br> * `win64` for you game client exectuable </br></br> **`<configuration>`**</br> Use the [Unreal build configuration](https://docs.unrealengine.com/en-us/Programming/Development/BuildConfigurations) `Development` whih is the one you usually use during game development. </br></br>* `--skip-codegen` is optional, add this if you want the build script to _NOT_ generate [schema]({{urlRoot}}/content/glossary#schema-generation). |

<br/>

<br/>------------<br/>
_2019-06-27 Page updated with editorial review_
<br/>
<br/>
