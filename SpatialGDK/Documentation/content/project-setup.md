<!--- DO NOT DELETE - I am a page linked to from the Third-Person Shooter game
(https://github.com/spatialos/UnrealGDKThirdPersonShooter) and Test Suite (https://github.com/spatialos/UnrealGDKTestSuite) repos --->

# Set up the Third-Person Shooter game or Test Suite 

Use this guide to set up the [Third-Person Shooter game](https://github.com/spatialos/UnrealGDKThirdPersonShooter) or the [Test Suite](https://github.com/spatialos/UnrealGDKTestSuite) GDK projects. 

Before starting this, you need to have followed:

* [Getting started: 1 - Dependencies]({{urlRoot}}/content/get-started/dependencies)
* [Getting started: 2 - Set up the fork and plugin]({{urlRoot}}/content/get-started/build-unreal-fork)

### Terms used on this page

- `<GameRoot>` - The directory that contains your project’s .uproject file and Source folder.
- `<YourProject>` - The name of your project and .uproject file (for example, `\<GameRoot>\YourProject.uproject`).

To open these projects, you must clone the repositories and build the project in Visual Studio.

### Clone the repositories

First, clone the repository for the project you want to set up: 

1. Clone the project by running 
    * (HTTPS) `git clone <GitHub URL>.git`
    Where `<GitHub URL>` is the URL of the repository you want to clone. For example:
    `git clone https://github.com/spatialos/UnrealGDKThirdPersonShooter.git`

1. Navigate into `<GameRoot>\Game\` and create a `Plugins` folder.
1.  In a command line  window, navigate to `<GameRoot>\Game\Plugins` and clone the [GDK for Unreal](https://github.com/spatialos/UnrealGDK) repository by running either:
    * (HTTPS) `git clone https://github.com/spatialos/UnrealGDK.git`
    * (SSH) `git clone git@github.com:spatialos/UnrealGDK.git`

**Note** You need to ensure that the root folder of the GDK for Unreal repository is called `UnrealGDK` so its path is: `<GameRoot>\Game\Plugins\UnrealGDK\...`

### Build the project 

To use the project, you must build the GDK for Unreal module dependencies. To do this:

1. Open File Explorer, navigate to the root directory of the GDK for Unreal repository (`<GameRoot>\Game\Plugins\UnrealGDK\...`), and double-click `Setup.bat`. If you haven’t already signed into your SpatialOS account, the SpatialOS developer website may prompt you to sign in.
2. In File Explorer, navigate to your `<GameRoot>\Game` directory.
3. Right-click on **`<YourProject>`.uproject** and select **Switch Unreal Engine Version** then select the Unreal Engine you cloned earlier. <br/>
   ![img]({{assetRoot}}assets/screen-grabs/select-unreal-engine.png)<br/>
   _Image: The Select Unreal Engine Version window_<br/><br/>
4. Right-click **`<YourProject>`.uproject** and select **Generate Visual Studio Project files**.
5. In the same directory, double-click **`<YourProject>`.sln** to open it with Visual Studio.
6. In the Solution Explorer window, right-click on **`<YourProject>`** and select **Build**.
7. When Visual Studio has finished building your project, right-click on **`<YourProject>`** and select **Set as StartUp Project**.
8. Build and open your project in Unreal Editor. To do this: Either press F5 on your keyboard or, in the Visual Studio toolbar, select *Local Windows Debugger*. This opens your project in Unreal Editor. 
   ![Visual Studio toolbar]({{assetRoot}}assets/set-up-template/template-vs-toolbar.png)<br/>
   _Image: The Visual Studio toolbar_ <br/>
   **Note:** Ensure that your Visual Studio Solution Configuration is set to **Development Editor**. <br/>

You can now launch a local or cloud deployment, or edit the project. 

For information about launching a local or cloud deployment, see the [Starter Template]({{urlRoot}}/content/get-started/starter-template/get-started-template-local) or [Example Project]({{urlRoot}}/content/get-started/example-project/exampleproject-local-deployment) guides. 
