<%(TOC)%>
# Set up Visual Studio for worker debugging

To debug workers from Visual Studio, you need to set up the commands in VS to start up a local SpatialOS deployment in Unreal Editor for you, running in the background. Once you have set this up, you can run server-workers and game clients direct from Visual Studio, without switching to the Editor.

> **Note:** To debug a game client, you must also set up your project to auto-start a server-worker type in a local deployment, this is so the game client has a server-worker instance to compute the server side of the game. For guidance on this, see the [Auto-start server-workers]({{urlRoot}}/content/workflows/autostart-server-workers).

In VS, right-click on your project name and select **Properties**.
In the left column, select **Debugging**.

<%(Lightbox image="{{assetRoot}}assets/workflows/vs-command-arguments.png")%>

Inside **Command Arguments** you see the following:
</br></br>
`"$(SolutionDir)$(ProjectName).uproject" -skipCompile`

Here, you need to add extra arguments that the local SpatialOS Runtime uses when you launch your server-worker instance or game client in a local deployment.

* Remove `-skipCompile`
* Specify a URL for a game client or a MapName for a server-worker instance. This must immediately follow the executable name. For example: `"$(SolutionDir)$(ProjectName).uproject" 127.0.0.1`.
* Specify a mode for the Editor; `-game` for a game client, `-server` for a server-worker instance.
* Specify the server-worker type or game client. For example:
  * Game client: `-workerType UnrealClient` </br>
  * Server worker: `-workerType UnrealWorker`

Now you can debug from VS. Full command arguments will look something like this:

**Client:** `"$(SolutionDir)$(ProjectName).uproject" 127.0.0.1 -game -workerType UnrealClient`
**Server:** `"$(SolutionDir)$(ProjectName).uproject" MapName -server -workerType UnrealWorker`

**Notes:**

* To output logs to an external console window as well as the Visual Studio output window, add `-log` to your arguments.  
* To add any additional arguments, see [Unreal documentation](https://docs.unrealengine.com/en-us/Programming/Basics/CommandLineArguments).





<br/>------------<br/>
_2019-07-19: Page added with editorial review(s)_
