<%(TOC max="3")%>

# Manual-install: Example Project setup

When you follow [Get started: 2 - Set up the fork and plugin]({{urlRoot}}/content/get-started/build-unreal-fork.md), we recommend you [auto-install]({{urlRoot}}/content/get-started/build-unreal-fork#step-5-clone-and-install-the-plugin) the fork and plugin. However, you have the option to manually install the the UE fork and plugin by following the [Manual-install: UE fork and plugin]({{urlRoot}}/content/get-started/manual-engine-build) guide.

If you followed the manual-install guide, follow the steps below to set up the Example Project. After this, you can return to [Example Project - 2: Launch a local deployment]({{urlRoot}}/content/get-started/example-project/exampleproject-local-deployment). 


## Example Project - Set up (manual-install only) 
> If you used **auto-install** (by running `InstallGDK.bat`) as part of _Get Started: 2 - Set up the fork and plugin_, follow the standard [setup instructions]({{urlRoot}}/content/get-started/example-project/exampleproject-setup). Do not follow the instructions below.

To run the GDK and the Example Project, you need to:

* Sign up for a SpatialOS account (or make sure you are logged in)
* Clone the Example Project repository
* Clone the GDK plugin repository into your Example Project
* Build and launch the Example Project

### Step 1: Sign up or log in

If you have not signed up for a SpatialOS account, you can do this [here](https://improbable.io/get-spatialos).
<br/>
If you have already signed up, make sure you are logged into [Improbable.io](https://improbable.io). If you are logged in, you should see your picture in the top right of this page; if you are not logged in, select __Sign in__ at the top of this page and follow the instructions.

### Step 2: Clone the Example Project repository

[block:html]
{
  "html": "<div class=\"wrap-collapsible\">\n  <input id=\"collapsible\" class=\"toggle\" type=\"checkbox\">
  <label for=\"collapsible\" class=\"lbl-toggle\">Using the command line </label>\n  <div class=\"collapsible-content\">\n    <div class=\"content-inner\">\n      <p>\n </p>\n    </div>\n  </div>\n</div>"
}
[/block]


<button class=\"collapsible\">Using Github Desktop</button>
<div>


1. In GitHub Desktop, select **File** >  **Clone  Repository**.<br/>
1. In the Clone a repository window, select **URL.**<br/>
1. In the Repository URL field, enter this URL: `https://github.com/spatialos/UnrealGDKExampleProject.git`<br/>
1. In the **Local Path** field, enter a suitable directory path for this repository, or select **Choose…** to select a directory using File Explorer. <br/>
1. Select **Clone**. <br/>
![img]({{assetRoot}}assets/screen-grabs/github-desktop.png)<br/>
_Image: The Github Desktop Clone a repository window_<br/>

</div>

> **TIP:** Clone the Example project into your root directory to avoid file path length errors. For example: `C:\GitHub\UnrealGDKExampleProject`.

### Step 3: Clone the GDK plugin repository

Next, you need to clone the GDK into your project. To do this: 

1. In File Explorer, navigate to the `UnrealGDKExampleProject\Game` directory and create a `Plugins` folder in this directory.
1. In a terminal window, navigate to the `UnrealGDKExampleProject\Game\Plugins` directory and clone the [GDK for Unreal](https://github.com/spatialos/UnrealGDK) repository by running either:

|  |  |
| ----- | ---- |
| HTTPS | `git clone https://github.com/spatialos/UnrealGDK.git` |
| SSH | `git clone git@github.com:spatialos/UnrealGDK.git`|

### Step 4: Build the dependencies and launch the project

To use the Example project, you must build the GDK for Unreal module dependencies. To do this:

1. Open File Explorer, navigate to the root directory of the GDK for Unreal repository (`UnrealGDKExampleProject\Game\Plugins\UnrealGDK\...`), and double-click `Setup.bat`. If you haven’t already signed into your SpatialOS account, the SpatialOS developer website may prompt you to sign in.
1. In File Explorer, navigate to your `UnrealGDKExampleProject\Game` directory.
1. Right-click on **GDKShooter.uproject** and select **Switch Unreal Engine Version** then select the Unreal Engine you cloned earlier. <br/>
    ![img]({{assetRoot}}assets/screen-grabs/select-unreal-engine.png)<br/>
    _Image: The Select Unreal Engine Version window_<br/><br/>
1. Right-click **GDKShooter.uproject** and select **Generate Visual Studio Project files**.
1. In the same directory, double-click **GDKShooter.sln** to open it with Visual Studio.
1. In the Solution Explorer window, right-click on **GDKShooter** and select **Build**.
1. When Visual Studio has finished building your project, right-click on **GDKShooter** and select **Set as StartUp Project**.
1. Build and open your project in the Unreal Editor. To do this: Either press F5 on your keyboard or, in the Visual Studio toolbar, select *Local Windows Debugger*.<br/>

   ![Visual Studio toolbar]({{assetRoot}}assets/set-up-template/template-vs-toolbar.png)<br/>
   _Image: The Visual Studio toolbar_ <br/>
   **Note:** Ensure that your Visual Studio Solution Configuration is set to **Development Editor**. <br/><br/><br/>
   ![img]({{assetRoot}}assets/example-project/example-project-editor.png)<br/>
   _Image: The Example Project in the Unreal Editor_<br/><br/>

#### **> Next:** [2: Launch a local deployment]({{urlRoot}}/content/get-started/example-project/exampleproject-local-deployment) 

<br/>------<br/>
_2019-07-22 Page updated with editorial review: updated navigation_</br>
_2019-07-22 Page added with limited editorial review_
