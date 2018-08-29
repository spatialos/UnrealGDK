> This [pre-alpha](https://docs.improbable.io/reference/13.1/shared/release-policy#maturity-stages) release of the SpatialOS Unreal GDK is for evaluation and feedback purposes only, with limited documentation - see the guidance on [Recommended use]({{urlRoot}}/index#recommended-use)

# Helper scripts

| Helper script | Parameters | Description |
| --- | --- | --- |
| `Game\Scripts\Codegen.bat` | None | Generates SpatialOS C++ Worker code from the generated schema and type-bindings. |
| `Game\Scripts\BuildWorkerConfig.bat` | None | Generates launch configurations for the SpatialOS Runtime. You need to re-run this if you've made changes to either of these files: </br> - `spatialos.UnrealClient.worker.json`</br> - `spatialos.UnrealWorker.worker.json` </br></br>You can find the generated launch configuration(s) in `<ProjectName>\spatial\build\assembly\<WorkerName>\*`|
| `Game\Scripts\BuildWorker.bat` | `<target> <platform> <configuration> <YourGame>.uproject [--skip-codegen]` | Example: </br> `Game\Scripts\BuildWorker.bat ExampleGameEditor Win64 Development ExampleGame.uproject` </br></br> Build, cook and zip your Unreal server-workers and client-workers for use with a SpatialOS cloud deployment (uploaded using [`spatial cloud upload`](https://docs.improbable.io/reference/13.1/shared/deploy/deploy-cloud)). </br></br>  The following `<target>`s  generate zipped workers: </br> - `<YourGame>` </br> - `<YourGame>Server` </br> - `<YourGame>Editor` </br></br> Any other `<target>` passes all arguments to `Engine\Build\BatchFiles\BuildWorker.bat` with no cooking or zipping performed.|
