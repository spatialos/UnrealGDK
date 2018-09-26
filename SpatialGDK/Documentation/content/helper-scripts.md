<%(Callout type="warn" message="This [pre-alpha](https://docs.improbable.io/reference/latest/shared/release-policy#maturity-stages) release of the SpatialOS GDK for Unreal is for evaluation and feedback purposes only, with limited documentation - see the guidance on [Recommended use]({{urlRoot}}/index#recommended-use)")%>

# Helper scripts

These scripts are located in your UnrealGDK repo under `SpatialGDK\Build\Scripts\`

| Helper script | Parameters | Description |
| --- | --- | --- |
| `BuildWorker.bat` | `<target> <platform> <configuration> <YourGame>.uproject [--skip-codegen]` | Example: </br> `BuildWorker.bat ExampleGameEditor Win64 Development ExampleGame.uproject` </br></br> Build, cook and zip your Unreal server-workers and client-workers for use with a SpatialOS cloud deployment (uploaded using [`spatial cloud upload`](https://docs.improbable.io/reference/latest/shared/deploy/deploy-cloud)). </br></br>  The following `<target>`s  generate zipped workers: </br> - `<YourGame>` </br> - `<YourGame>Server` </br> - `<YourGame>Editor` </br></br> Any other `<target>` passes all arguments to `Engine\Build\BatchFiles\Build.bat` with no cooking or zipping performed.|
