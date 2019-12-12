

## \[Experimental\] Database sync worker

## 4. Testing your changes

At this point your project is ready to run, you just need to do a couple of config steps for it to work correctly with the Database Sync Worker and the database.

### 1. Setting up the snapshot

You have written code to send commands to a specific entity that contains the `DatabaseSyncService` SpatialOS component, but you have never created that entity.

Because the components of that entity have nothing to do with Unreal (they do not map to `ActorComponents`), you cannot just create an Actor to represent that entity. The entity needs to be created directly in the Snapshot (or if you have any other non Unreal Worker, it could create the entity itself).

At the moment there is not a simple tool to modify Snapshots and you’d need to do it yourself by using the [Platform SDK](https://docs.improbable.io/reference/latest/platform-sdk/introduction). For this example, we provide a Snapshot with the entity already included.

To use the snapshot, copy `default.snapshot` from `spatial/provided` into `spatial/snapshots`.

### 2. Preparing the database

The Database Sync Worker is set by default to use certain hierarchical database structure. We provide a tool to do automatically set up the database accordingly.

Run `spatial/workers/database-sync-worker/scripts/reset-database.ps1` (or`.sh`) and it will set up the database correctly.

For the Database Sync Worker to allow the UnrealWorker access to the database, there needs to be already an entry in the database with the profile for the UnrealWorker before you can use this so you will need to create it.

Open [pgAdmin 4](https://www.pgadmin.org/) (or an equivalent client for Postgres databases)  and in the `items` table, run the following query:

```
INSERT INTO items (name, count, path) VALUES ('UnrealWorker', 1, 'profiles.UnrealWorker')
```

### 3. Launch configuration

Next, build the project from Visual Studio and be sure everything compiles correctly.

[block:callout]
{
  "type": "tip",
  "body": "If you have any issues, you can compare your code to a final working version of the tutorial in the `feature/dbsync_worker_tutorial_finish` branch of the Example Project."
}
[/block]

The default Example Project launch configuration only starts UnrealWorkers. The launch configuration now also needs to start the Database Sync Worker you added. We provide a launch config that does this, but you can learn more about how to change those config files [here](https://docs.improbable.io/reference/latest/shared/project-layout/launch-config#launch-configuration-file).

For now, copy `default.launch.json` from `spatial/provided` into `spatial`.

By default, SpatialOS GDK settings are set to generate a default launch config when you launch the game. Here, you need to modify config setting in the Unreal to not do it anymore since you want to use the config file we just provided. To do so, open the SpatialOS GDK Editor Settings in Unreal, and uncheck `Generate default launch config`.

<%(Lightbox image="{{assetRoot}}assets/dbsync/editor-settings-generate-launch-config.png")%>

### 4. Running the game

At this point, all is ready, so in the Unreal Editor:

1. Generate Schema (this is required to create the descriptor files for the schema we imported from outside the Unreal project and would not have been generated when you generated Schema in step 1)

2. Open the Play drop-down menu. Under Multiplayer Options, set the number of players to 2 or 3 and hit play

As you kill players, you will the “All Time” columns change.

Now, stop play, stop the deployment, and restart Unreal Engine. Run the game as previously - you should see that even with 0 kills / deaths in current game, the *All Time* Kills and *All Time Deaths* are populated, because you are getting the information from the database.

<%(Lightbox image="{{assetRoot}}assets/dbsync/kd-populated.png")%>

You can confirm this by running `select * from items` in the database itself and see the contents.

### 5. Live editing

As we pointed before, the ability to live-edit the database and have those changes reflected in the game is a key aspect of the Database Sync Worker.

To try this out, while you have the game running, run some queries in the database to modify the values. For instance, run
`UPDATE items SET count = 999 WHERE path ~ 'profiles.UnrealWorker.players.256.score.AllTimeDeaths'` (replace 256 with the ID of the player you want to modify).

You will see that the scores change automatically in the game too.

<%(Lightbox image="{{assetRoot}}assets/dbsync/dbsync-kd-exampleproject.gif")%>

### 6. Congratulations

Congratulations! You have a working implementation of the Database Sync Worker, synchronising data from a GDK project to an external Postgres database. We hope this was a useful basis for an implementation in your project.

<br/>
<br/>------<br/>
_2019-07-31 Page added with limited editorial review_
[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1248)

