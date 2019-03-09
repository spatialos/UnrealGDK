<%(TOC)%>
# Helper scripts

These scripts are located under `Plugins\UnrealGDK\SpatialGDK\Build\Scripts\` directory of your game.

| Helper script | Parameters | Description |
| --- | --- | --- |
| `BuildWorker.bat` | `<target> <platform> <configuration> <YourGame>.uproject [--skip-codegen]` | Example: </br> `BuildWorker.bat ExampleGameEditor Win64 Development ExampleGame.uproject` </br></br> Build, cook and zip your Unreal server-workers and client-workers for use with a SpatialOS cloud deployment (uploaded using [`spatial cloud upload`](https://docs.improbable.io/reference/latest/shared/deploy/deploy-cloud)). </br></br>  The following `<target>`s  generate zipped workers: </br> - `<YourGame>` </br> - `<YourGame>Server` </br> - `<YourGame>Editor` </br></br> Any other `<target>` passes all arguments to `Engine\Build\BatchFiles\Build.bat` with no cooking or zipping performed.|
