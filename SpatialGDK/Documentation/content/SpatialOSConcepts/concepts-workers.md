# Workers
The [SpatialOS Runtime](#spatialos-runtime) manages the [SpatialOS world](#spatialos-world): it keeps track of all the [SpatialOS entities](#spatialos-entity) and their
[SpatialOS components](#spatialos-component). But on its own, it doesn’t make any changes to the world.

Workers are programs that connect to a SpatialOS world. They perform the computation associated with a world: they can read what’s happening, watch for changes, and make changes of their own.

There are two types of workers; server-workers and client-workers. 

A server-worker approximates to a server in native Unreal networking but, unlike Unreal networking, in SpatialOS you can have have more than one server-worker.

## Server-worker

A server-worker is a [worker](#workers) whose lifecycle is managed by SpatialOS. When running a [deployment](#deployment), the SpatialOS Runtime starts and stops server-workers based on your chosen [load balancing](https://docs.improbable.io/reference/latest/shared/glossary#load-balancing) configuration.

    You usually set up server-workers to implement game logic and physics simulation. You can have one server-worker connected to your [deployment](#deployment), or dozens, depending on the size and complexity of your [SpatialOS world](#spatialos-world).
    If you run a [local deployment](#deployment), the server-workers run on your development computer. If you run a [cloud deployment](#deployment), the server-workers run in the cloud.
## Client-worker

    While the lifecycle of a server-worker is managed by the SpatialOS Runtime, the lifecycle of a client-worker is managed by the game client.
    You usually set up client-workers to visualize what’s happening in the [SpatialOS world](#spatialos-world). They also deal with player input.

    > Related:
    >
    > * [External worker (client-worker) launch configuration](https://docs.improbable.io/reference/latest/shared/worker-configuration/launch-configuration#external-worker-launch-configuration)

## More about workers

In order to achieve huge scale, SpatialOS divides up the SpatialOS entities in the world between workers, balancing the work so none of them are overloaded. For each SpatialOS entity in the world, it decides which worker should have [write access](#authority) to each SpatialOS component on the SpatialOS entity. To prevent multiple workers writing to a component at the same time, only one worker at a time can have write access to a SpatialOS component.

As the world changes over time, the position of SpatialOS entities and the amount of updates associated with them changes. Server-workers report back to SpatialOS how much load they're under, and SpatialOS adjusts which workers have write access to components on which SpatialOS entities. SpatialOS then starts up new workers when needed. This is [load balancing](https://docs.improbable.io/reference/latest/shared/worker-configuration/loadbalancer-config).
Around the SpatialOS entities which they have write access to, every worker has an area of the world they are [interested in](#interest).

A worker can read the current state of components of the SpatialOS entities within this area, and SpatialOS sends [updates and messages](https://docs.improbable.io/reference/latest/shared/glossary#sending-an-update) about these SpatialOS entities to the worker.

If the worker has write access to a SpatialOS component, it can [send updates and messages](https://docs.improbable.io/reference/latest/shared/glossary#sending-an-update):
it can update [properties](https://docs.improbable.io/reference/latest/shared/glossary#property), send and handle [commands](https://docs.improbable.io/reference/latest/shared/glossary#command) and trigger [events](https://docs.improbable.io/reference/latest/shared/glossary#event).

> Related:
>
> * [Concepts: Workers and load balancing](https://docs.improbable.io/reference/latest/shared/concepts/workers-load-balancing)

## Worker configuration
The worker configuration is how you set up your workers. It is represented in the worker configuration file.
See [workers](#workers) and [worker configuration file](#worker-configuration-file).

### Worker configuration file

> First, see [workers](#workers).

Each worker needs a worker configuration file. This file tells SpatialOS how to build, launch, and interact with the workers.
The file’s name must be `spatialos.<worker_type>.worker.json`: for example, `spatialos.MyWorkerType.worker.json`.
Once you’ve chosen a label for the worker type (for example, myWorkerType), you use this exact label consistently throughout your project to identify this worker type.

> Related:
>
> [Worker configuration file `worker.json` (SpatialOS documenation)](https://docs.improbable.io/reference/latest/shared/worker-configuration/worker-configuration)