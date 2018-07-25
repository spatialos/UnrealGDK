## Helper scripts

| Helper script | Parameters | Description |
| --- | --- | --- |
| `Game\Scripts\Codegen.bat` | None | Generates SpatialOS C++ Worker code from the generated schema and type-bindings. |
| `Game\Scripts\BuildWorkerConfig.bat` | None | Generates launch configurations for the SpatialOS Runtime. You need to re-run this if you've made changes to either of these files: <br> - `spatialos.UnrealClient.worker.json`<br> - `spatialos.UnrealWorker.worker.json` |
| `Game\Scripts\Build.bat` | `<target> <platform> <configuration> TestSuite.uproject [--skip-codegen]` | Example: <br> `Game\Scripts\Build.bat TestSuiteEditor Win64 Development TestSuite.uproject` <br><br> Build, cook and zip your Unreal server workers and client workers for use with a SpatialOS cloud deployment (uploaded using [`spatial cloud upload`](https://docs.improbable.io/reference/13.1/shared/deploy/deploy-cloud)). <br><br>  The following `<target>`s  generate zipped workers: <br> - `TestSuiteEditor` <br> - `TestSuite` <br> - `TestSuiteServer` <br><br> Any other `<target>` passes all arguments to `Engine\Build\BatchFiles\Build.bat` - either no cooking or zipping performed.|