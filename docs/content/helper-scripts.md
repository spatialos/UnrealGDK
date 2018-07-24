## Helper scripts

| Helper script | Parameters | Description |
| --- | --- | --- |
| `Game\Scripts\Codegen.bat` | None | Generates code from the project schema. |
| `Game\Scripts\BuildWorkerConfig.bat` | None | Generates launch configurations for the SpatialOS Runtime. You need to re-run this if you've made changes to either of these files: <br> - `spatialos.UnrealClient.worker.json`<br> - `spatialos.UnrealWorker.worker.json` |
| `Game\Scripts\Build.bat` | `<target> <platform> <configuration> TestSuite.uproject [--skip-codegen]` | Example: <br> `Game\Scripts\Build.bat TestSuiteEditor Win64 Development TestSuite.uproject` <br><br> Build, cook and zip special workers for use with: <br> - `spatial upload` <br> - `spatial local launch` <br><br>  The following `<target>`s  generate zipped workers: <br> - `TestSuiteEditor` <br> - `TestSuite` <br> - `TestSuiteServer` <br><br> Any other `<target>` passes all arguments to `Engine\Build\BatchFiles\Build.bat` - either no cooking or zipping performed.|