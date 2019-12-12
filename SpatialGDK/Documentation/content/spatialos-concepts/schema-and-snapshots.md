
# SpatialOS concepts: schema and snapshots

## Schema

Schema is a set of definitions which represent your game’s objects in SpatialOS as entities. Schema is defined in .schema files and written in schemalang by the GDK.</br>
Select **Schema** from the GDK toolbar and the GDK generates schema files and their contents for you, so you do not have to write or edit schema files manually.

SpatialOS uses schema to generate APIs specific to the entity components in your project. You can then use these APIs in your game's [worker types]({{urlRoot}}/content/glossary#worker-types-and-worker-instances) so their instances can interact with [entity components]({{urlRoot}}/content/glossary#spatialos-component).

You can find out how to use schema in the [schema reference documentation]({{urlRoot}}/content/how-to-use-schema)

## Snapshots

A snapshot is a representation of the state of a [SpatialOS world]({{urlRoot}}/content/glossary#spatialos-world) at a given point in time. It stores each [persistent]({{urlRoot}}/content/glossary#persistence) [entity]({{urlRoot}}/content/glossary#entity) and the values of their [SpatialOS components]({{urlRoot}}/content/glossary#spatialos-component)' [properties](https://docs.improbable.io/reference/latest/shared/glossary#property).

You can find out how to use snapshots in the [snapshot reference documentation]({{urlRoot}}/content/how-to-use-snapshots).

<br/>

<br/>------------<br/>
_2019-06-27 Page updated with limited editorial review_
<br/>	<br/>
