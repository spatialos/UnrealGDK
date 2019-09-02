# Database sync worker

## 1. Set up

### 1. Set up Postgres

To use this tutorial, you must install [Postgresql 11.x](https://www.postgresql.org/) - the database that the Database Sync Worker integrates with.

You can download it from [here](https://postgresql.org/download/windows). Once downloaded, set its password to `DO_NOT_USE_IN_PRODUCTION`.

<%(Callout type="tip" message="If you install a Postgresql version lower than 11, you may see an error similar to `ERROR:  record old is not assigned yet` when trying to modify the database. If this happens, uninstall Postgresql and re-install a 11.x version.")%>

### 2. Set up the Example Project

If you followed [Get started 1 - Get the dependencies]({{urlRoot}}/content/get-started/dependencies), and [Get started 2 - Set up the fork and plugin]({{urlRoot}}/content/get-started/build-unreal-fork), you will have a fully set up Example Project already.

For this tutorial, you will use the `feature/dbsync_worker_tutorial_start` branch rather than the default `release` branch. To switch branches, run `git checkout origin/feature/dbsync_worker_tutorial_start`.

Then, open the Visual Studio solution and build the project.

<%(Callout type="tip" message="If you just want to see the final result, you can find the completed source version of this tutorial in the `feature/dbsync_worker_tutorial_finish` branch of the Example Project. While the game code will be complete, _**you must still follow the remaining steps on this page.**_")%>

Once built, open the project in the Unreal Editor, generate Schema, create a Snapshot and start a deployment with a couple of players in `New Editor Window`.

As you kill other players, you will see that the score screen has "All Time" columns updated alongside the per-game `Kill/Deaths` ones.

![K/D counts getting populated]({{assetRoot}}assets/dbsync/kd-counts-2-0.png)

However, if you stop and start again, the "All Time" columns are reset to 0 because the information is not being stored anywhere:

![K/D counts getting reset]({{assetRoot}}assets/dbsync/kd-counts-0-0.png)

The tracking of those stats is done in `DeathmatchScoreComponent.cpp`: you can have a look at the code there and will see the places where you will need to store and read the data from the database using the Database Sync Worker.

### 3. Clone and build the Database Sync Worker

We are going to add the Database Sync Worker into the Example Project as another SpatialOS worker. To do so, clone [the latest tag](https://github.com/spatialos/database-sync-worker/tags) into the `/spatial/workers` folder of the Example Project.

Then, run the following:

1. `spatial/workers/database-sync-worker/scripts/build-nuget-packages.ps1` (or `.sh`).
2. `spatial/workers/database-sync-worker/scripts/publish-windows-workers.ps1` (or `.sh`) which will create the executable for the worker to be run locally.

To be run as a [managed worker](https://docs.improbable.io/reference/latest/shared/design/design-workers#managed-workers) by SpatialOS, the executable needs to be packaged in a specific way (described in the config file). To do so, go to `spatial/workers/database-sync-worker/Workers/DatabaseSyncWorker/bin/x64/Release/netcoreapp2.2/win-x64/publish` and zip the entire contents of that folder (but not the folder itself) into a zip file called `DatabaseSyncWorker@Windows.zip` and move it to `spatial/build/assembly/worker` (create the folder if it doesn’t exist).

Within the Example Project, copy the `spatial/provided/spatialos.DatabaseSyncWorker.worker.json` file to `spatial/workers/database-sync-worker`.
This step is required because the Database Sync Worker uses a different [SpatialOS project layout](https://docs.improbable.io/reference/latest/shared/project-layout/files-and-directories#layout-of-a-spatialos-project) than GDK project. In the future, both will use the Flexible Project Layout.

### 4. Bringing the Database Sync Worker schema into the Example Project

The Database Sync Worker uses SpatialOS components, commands and events that are defined in `.schema` files in that project.

To communicate with that worker, you need the UnrealWorker to send commands and receive events defined in those schema files.

1. Copy the `database_sync.schema` file from `spatial/workers/database-sync-worker/Improbable/DatabaseSync/Improbable.DatabaseSync.Schema/schema/improbable/database_sync` into `spatial/schema/dbsync` (you will need to create this folder).
1. Copy the `postgres.schema` file from `spatial/workers/database-sync-worker/Improbable/Postgres/Improbable.Postgres.Schema/schema/improbable/postgres` into `spatial/schema/dbsync`
1. Because `database_sync.schema` references `postgres.schema`, you need to modify `database_sync.schema`’s `postgres.schema` import statement to look for the correct folder:  `import "dbsync/postgres.schema";`
1. Open a console in the root of the game project.
   1. In the console, run the code generator tool: `../../Engine/Plugins/UnrealGDK/SpatialGDK/Build/Scripts/ExternalSchemaCodegen.bat "%cd%" "spatial/schema/dbsync" "Game/Source/GDKShooter/ExternalSchemaCodegen"`
   1. For more information on this tool, including the parameters it accepts, see [this reference page]({{urlRoot}}/content/apis-and-helper-scripts/helper-scripts).
1. Right click on `Game/GDKShooter.uproject` and `Generate Visual Studio Files` for your newly created code to be included in the project.

You've now set up up the Database Sync worker - let's start using it!

</br>
#### **> Next:** [2: Communicating with the Database Sync Worker]({{urlRoot}}/content/tutorials/dbsync/tutorial-dbsync-communicating)
</br>

<br/>------<br/>
_2019-09-02 Page updated with limited editorial review: point to the latest tagged version of the repository_
_2019-07-31 Page added with limited editorial review_
[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1248)
