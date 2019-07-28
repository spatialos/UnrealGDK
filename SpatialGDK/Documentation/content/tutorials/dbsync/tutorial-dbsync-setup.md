# Database Sync Worker Tutorial

## 1. Set up

### 1. Set up Postgres

To use this tutorial, you must also install [Postgresql](https://www.postgresql.org/) - the database that the Database Sync Worker integrates with. You can download it from [here](https://postgresql.org/download/windows). Once downloaded, set its password to `DO_NOT_USE_IN_PRODUCTION`.

### 2. Set up the Example Project

If you followed [Get started 1 - Get the dependencies]({{urlRoot}}/content/get-started/dependencies), and [Get started 2 - Get and build the SpatialOS Unreal Engine Fork]({{urlRoot}}/content/get-started/build-unreal-fork), you will have a fully set up Example Project already. 

For this tutorial, you will use the `dbsync_worker_tutorial_start` branch rather than the default `release` branch. To switch branches, run `git checkout origin/db_worker_tutorial_start`.

Then, open the Visual Studio solution and build the project.

Once built, open the project in the Unreal Editor, generate Schema, create a Snapshot and start a deployment with a couple of players in `New Editor Window`.

As you kill other players, you will see that the score screen has "All Time" columns updated alongside the per-game `Kill/Deaths` ones. 

![K/D counts getting populated]({{assetRoot}}assets/dbsync/kd-counts-2-0.png)

However, if you stop and start again, the "All Time" columns are reset to 0 because the information is not being stored anywhere:

![K/D counts getting reset]({{assetRoot}}assets/dbsync/kd-counts-0-0.png)

The tracking of those stats is done in `DeathScoreComponent.cpp`: you can have a look at the code there and will see the places where you will need to store and read the data from the Database using the Database Sync Worker.

### 3. Clone and build the Database Sync Worker

We are going to add the Database Sync Worker into the Example Project as another SpatialOS worker. To do so, clone [this repository](https://github.com/spatialos/database_sync_worker) into the `/spatial/workers` folder of the Example Project.

Then, in `spatial/workers/database_sync_worker/scripts` run `build-nuget-packages.ps1` (or `.sh` if you are using something like Git Bash).

Once that is completed, run `publish-windows-workers.ps1` (or `.sh`) which will create the executable for the worker to be run locally.

To be run as a [managed worker](https://docs.improbable.io/reference/latest/shared/design/design-workers#managed-workers) by SpatialOS, the executable needs to be packaged in a specific way (described in the config file). To do so, go to `spatial/workers/database_sync_worker/Workers/DatabaseSyncWorker/bin/x64/Release/netcoreapp2.2/win-x64/publish` and zip the whole content of that folder (but not the folder itself) into a zip file called `DatabaseSyncWorker@Windows.zip` and move it to `spatial/build/assembly/worker` (create the folder if it doesn’t exist).

Within the Example Project, copy the `spatial/provided/spatialos.DatabaseSyncWorker.worker.json` file to `spatial/workers/database_sync_worker`. This step is required because the Database Sync Worker uses a different [SpatialOS project layout](https://docs.improbable.io/reference/layout/shared/project-layout/files-and-directories) than GDK project. In the future they both use the Flexible Project Layout. 

You've now set up up the Database Sync worker - let's start using it!

</br>
### **> Next:** [2: Communicating with the Database Sync Worker]({{urlRoot}}/content/tutorials/dbsync/tutorial-dbsync-communicating)
</br>

<br/>------<br/>
_2019-07-31 Page added with limited editorial review_
[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1248)

