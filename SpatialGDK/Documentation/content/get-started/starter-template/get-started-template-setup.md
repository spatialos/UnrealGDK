<%(TOC max="3")%>

# The Starter Template

> **Manual-install:** If you followed the [manual-install guide]({{urlRoot}}/content/get-started/manual-engine-build) when you set up the fork and plugin (that is, you did NOT run `InstallGDK.bat`), do not follow the instructions below. Instead, follow the [Manual-install: Example Project setup]({{urlRoot}}/content/get-started/starter-template/get-started-template-setup-manual) guide.

## 1. Set up

To use the Starter Template, you must complete the following steps:

1. Create a new project using the Starter Template
2. Build the GDK for Unreal module dependencies

### Step 1: Create a new project using the Starter Template

1. In File Explorer, navigate to `UnrealEngine\Engine\Binaries\Win64`and double-click **UE4Editor.exe** to open the Unreal Editor.
1. In Editor's [Project Browser](https://docs.unrealengine.com/en-us/Engine/Basics/Projects/Browser) window, select the **New Project** tab and then the **C++ tab**. 
1. In this tab, select **SpatialOS GDK**. 
1. In the **Folder** field, choose a suitable directory for your project.
1. In the **Name** field, enter a project name of your choice.
1. Select **Create Project**.

**Note:** When you create a project, Unreal Engine automatically creates a directory named after the project name you entered. This tutorial uses `<YourProject>` as the project name.

![The Unreal Engine Project Browser]({{assetRoot}}assets/set-up-template/template-project-browser.png)

_Image: The Unreal Engine Project Browser, with the project file path and project name highlighted._

After you have selected **Create Project**, Unreal Engine generates the necessary project files and directories, it then closes the Editor and automatically opens Visual Studio. 

After Visual Studio has opened, save your solution and close Visual Studio.

### Step 2: Build the dependencies 

To use the Starter Template, you must build the GDK for Unreal module dependencies and then add the plugin to your project. To do this: 

1. In File Explorer, navigate to your `<GameRoot>` directory, right-click `<YourProject>.uproject` and select **Generate Visual Studio project files.**
1. In the same directory, double-click `<YourProject>`.sln` to open it with Visual Studio.
1. In Visual Studio's Solution Explorer window, right-click on `<YourProject>` and select **Build**.
1. When Visual Studio has finished building your project, right-click on `<YourProject>` and select **Set as StartUp Project**.
1. Press F5 on your keyboard or select **Local Windows Debugger** in the Visual Studio toolbar to open your project in the Unreal Editor.<br/>
   ![Visual Studio toolbar]({{assetRoot}}assets/set-up-template/template-vs-toolbar.png)<br/>
   _Image: The Visual Studio toolbar_

**Note:** Ensure that your Visual Studio Solution configuration is set to **Development Editor**.

##### The toolbar
When you open the project in Unreal Editor, you will see there are additional options on the Unreal Editor toolbar and an additional Editor settings panels. You will use these GDK toolbar and some Editor settings options in later steps of this guide.

<%(Lightbox title="Toolbars" image="{{assetRoot}}assets/screen-grabs/toolbar/unreal-and-gdk-toolbar.png")%>

<%(#Expandable title="What's in the toolbar?")%>
Find out more about the SpatialOS version of the Unreal Editor toolbar: </br>* [Toolbars]({{urlRoot}}/content/unreal-editor-interface/toolbars) </br> * [Editor Settings panel]({{urlRoot}}/content/unreal-editor-interface/editor-settings) </br> * [SpatialOS Runtime Settings panel]({{urlRoot}}/content/unreal-editor-interface/runtime-settings)

<%(/Expandable)%>

</br></br>

#### **> Next:** [2: Launch a local deployment with multiple clients]({{urlRoot}}/content/get-started/starter-template/get-started-template-local)

<br/>

<br/>------------<br/>
_2019-08-12 Page updated with editorial review: updated navigation_<br/>
_2019-07-16 Page updated with editorial review_<br/>
