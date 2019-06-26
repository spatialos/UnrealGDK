# 3 - Set up a project: The Starter Template

## 1. Set up

### Step 1: Create a new project using the Starter Template

After [building the Unreal Engine fork]({{urlRoot}}/content/get-started/build-unreal-fork), in **File Explorer**, navigate to `UnrealEngine\Engine\Binaries\Win64`and double-click `UE4Editor.exe` to open the Unreal Editor.

1. In the [Project Browser](https://docs.unrealengine.com/en-us/Engine/Basics/Projects/Browser) window, select the **New Project** tab and then the **C++ tab**. 
2. In this tab, select **SpatialOS GDK Starter**. 
3. In the **Folder** field, choose a suitable directory for your project.
4. In the **Name** field, enter a project name of your choice.
5. Select **Create Project**.

**Note:** When you create a project, the Unreal Engine automatically creates a directory named after the project name you entered. This page uses `<YourProject>` as an example project name.

![The Unreal Engine Project Browser](C:/GitHub/Improbable/UnrealGDK/SpatialGDK/Documentation/content/get-started/%7B%7BassetRoot%7D%7Dassets/set-up-template/template-project-browser.png)

*Image: The Unreal Engine Project Browser, with the project file path and project name highlighted.*

After you have selected **Create Project**, the Unreal Engine generates the necessary project files and directories, it then closes the Editor and automatically opens Visual Studio. 

After Visual Studio has opened, save your solution then close Visual Studio before proceeding to the Clone the GDK step.

### Step 2: Clone the GDK

Now you need to clone the SpatialOS GDK for Unreal into your project. To do this: 

1. In **File Explorer**, navigate to the `<GameRoot>` directory and create a `Plugins` folder in this directory.
2. In a Git Bash terminal window, navigate to `<GameRoot>\Plugins` and clone the [GDK for Unreal](https://github.com/spatialos/UnrealGDK) repository by running either:
   - (HTTPS) `git clone https://github.com/spatialos/UnrealGDK.git`
   - (SSH) `git clone git@github.com:spatialos/UnrealGDK.git`

The GDK's [default branch (GitHub documentation)](https://help.github.com/en/articles/setting-the-default-branch) is `release`. This means that, at any point during the development of your game, you can get the latest release of the GDK by running `git pull` inside the `UnrealGDK` directory. When you pull the latest changes, you must also run `git pull` inside the `UnrealEngine` directory, so that your GDK and your Unreal Engine fork remain in sync.

**Note:** You need to ensure that the root directory of the GDK for Unreal repository is called `UnrealGDK` so the file path is: `<GameRoot>\Plugins\UnrealGDK\...`

### Step 3: Build the dependencies 

To use the Starter Template, you must build the GDK for Unreal module dependencies and then add the GDK to your project. To do this: 

1. Open **File Explorer**, navigate to the root directory of the GDK for Unreal repository (`<GameRoot>\Plugins\UnrealGDK\...`), and double-click **`Setup.bat`**. If you haven't already signed into your SpatialOS account, the SpatialOS developer website may prompt you to sign in. 
2. In **File Explorer**, navigate to your `<GameRoot>` directory, right-click `<YourProject>`.uproject and select Generate Visual Studio Project files.
3. In the same directory, double-click **`<YourProject>`.sln** to open it with Visual Studio.
4. In the Solution Explorer window, right-click on **`<YourProject>`** and select **Build**.
5. When Visual Studio has finished building your project, right-click on **`<YourProject>`** and select **Set as StartUp Project**.
6. Press F5 on your keyboard or select **Local Windows Debugger** in the Visual Studio toolbar to open your project in the Unreal Editor.<br/>
   ![Visual Studio toolbar](C:/GitHub/Improbable/UnrealGDK/SpatialGDK/Documentation/content/get-started/%7B%7BassetRoot%7D%7Dassets/set-up-template/template-vs-toolbar.png)<br/>
   _Image: The Visual Studio toolbar_

Note: Ensure that your Visual Studio Solution Configuration is set to **Development Editor**.

**> Next:** [2. Launch a local deployment with multiple clients](LinkTo)