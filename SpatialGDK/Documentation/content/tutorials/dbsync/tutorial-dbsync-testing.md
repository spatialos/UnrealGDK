<%(TOC)%>

# Database Sync Worker Tutorial

## 4. Testing your changes

At this point your project is ready to run, you just need to do a couple of config steps for it to work correctly with the Database Sync Worker and the database.

### 1. Setting up the snapshot

You have written code to send commands to a specific entity that contains the `DatabaseSyncService` component, but you have never created that entity.

Because the components of that entity have nothing to do with Unreal (they do not map to `ActorComponents`), you cannot just create an Actor to represent that entity. The entity needs to be created directly in the Snapshot (or if you have any other non Unreal Worker, it could create the entity itself).

At the moment there is not a simple tool to modify Snapshots and you’d need to do it yourself by using the [Platform SDK](https://docs.improbable.io/reference/latest/platform-sdk/introduction). For this example, we provide a Snapshot with the entity already included. 

To use the snapshot, copy `default.snapshot` from `spatial/provided` into `spatial/snapshots`.

### 2. Preparing the database

The Database Sync Worker is set by default to use certain hierarchical database structure. We provide a tool to do automatically set up the database accordingly.

Run `spatial/workers/database_sync_worker/scripts/reset-database.ps1` (or`.sh`) and it will set up the database correctly.

For the Database Sync Worker to allow the UnrealWorker access to the Database, there needs to be already an entry in the Database with the profile for the UnrealWorker before you can use this so you will need to create it.

Open [pgAdmin 4](https://www.pgadmin.org/) (or an equivalent client for Postgres databases)  and in the `items` table, run the following query: 

```
INSERT INTO items (name, count, path) VALUES ('UnrealWorker', 1, 'profiles.UnrealWorker')
```

### 3. Launch configuration

The default Example Project launch configiguration only starts UnrealWorkers. The launch configuration now also needs to start the Database Sync Worker you added. We provide a launch config that does this, but you can learn more about how to change those config files here. 

For now, copy `default.launch.json` from `spatial/provided` into `spatial`.



