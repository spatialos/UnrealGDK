

# Port your project to SpatialOS

## 1. Set up your project

To begin porting your project to the GDK, you need to:

* Modify your project's directory structure
* Copy the required SpatialOS folder and helper scripts from the Template project
* Configure the helper scripts

### Step 1: Modify your project's directory structure

The GDK uses a specific directory structure to run SpatialOS locally, and to build client-workers and server-workers. You must set up your project’s directory structure before using the GDK. 

To do this: Create a new empty directory to represent your `<ProjectRoot>` and move your `<GameRoot>` directory inside of it.  

Your project structure should be:  `\<ProjectRoot>\<GameRoot>\<YourProject>.uproject`<br/>

For example:
`\MyProject\Game\TP_SpatialGDK.uproject`

### Step 2: Copy files from the Template project

Your project needs some extra files and folders to run with the GDK. Copy these files from the template project that you set up earlier in the [Before you start]({{urlRoot}}/content/tutorials/tutorial-porting-guide#before-you-start) section.

To do this: Open File Explorer and navigate to the root of the `StarterTemplate` repository.

Copy all of the files and directories below to your `<ProjectRoot>`:  

* `\TP_SpatialGDK\spatial\` This folder contains the files that SpatialOS needs to run a deployment.
*  `\TP_SpatialGDK\LaunchClient.bat` 
  The GDK uses this script to launch a local Unreal client-worker and connects it to a local SpatialOS deployment. 
* `\TP_SpatialGDK\LaunchServer.bat`
  The GDK uses this script to launch a local Unreal server-worker and connects it to a local SpatialOS deployment.
* `\TP_SpatialGDK\ProjectPaths.bat`
  This script is used by the LaunchClient.bat and LaunchServer.bat scripts to specify the project environment when those scripts are run. 

   Your project's directory structure should now resemble:

[block:code]
{
  "codes": [
  {
      "code": "   \<ProjectRoot>\<GameRoot>\\n   \<ProjectRoot>\spatial\\n   \<ProjectRoot>\LaunchClient.bat \n   \<ProjectRoot>\LaunchServer.bat\n   \<ProjectRoot>\ProjectPaths.bat\n   Etc…",
      "language": "text"
    }
  ]
}
[/block]

**Note**: You must place the `spatial` directory in the directory above your `<GameRoot>`.
<br/>

### Step 3. Configure the ProjectPaths.bat GDK helper script.

You must edit the ProjectPaths.bat GDK helper scripts to launch local deployments of your project.

To do this: 

1. Open **`\<ProjectRoot>\ProjectPaths.bat`** in a text editor.  
1. In `set PROJECT_PATH=Game`, replace `Game` with your `<GameRoot>` folder name.  
1. In `set GAME_NAME= TP_SpatialGDK `, replace `TP_SpatialGDK ` with the name of your game's `.uproject` (`<YourProject>` [terms used in this guide]({{urlRoot}}/content/tutorials/tutorial-porting-guide#terms-used-in-this-guide)).  

**Note**: The helper scripts `LaunchClient.bat` and `LaunchServer.bat` will not work if you do not follow this step correctly. 

#### **> Next:** [2. Modify and build your project]({{urlRoot}}/content/tutorials/porting-guide/tutorial-portingguide-build)

<br/>

<br/>------------<br/>2019-07-16 Page updated with editorial review.<br/>
