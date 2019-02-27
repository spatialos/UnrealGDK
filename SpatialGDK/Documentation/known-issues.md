# GDK for Unreal known issues

Known issue = any major user-facing bug or lack of user-facing feature that:
1. diverges from vanilla Unreal design or implementation (e.g. ordering of reliable RPCs), **OR**
1. diverges from user expectations from a SpatialOS project (e.g. interacting across worker boundaries)

| Issue | Date added | Ticket | Workaround? |
|-------|-------------------|--------|-------------|
| Order of reliable RPCs is not respected in case of unresolved UObject* parameter. | 2018-06-14 | [UNR-363](https://improbableio.atlassian.net/browse/UNR-336) | Potential user-code workaround to explicitly enforce the order of RPCs. |
| Dynamic component replication. | 2018-07-03 | [UNR-366](https://improbableio.atlassian.net/browse/UNR-366) | Mimic behavior with static components. |
| Schema generator fails when the destination file is locked. The workflow is less than ideal as you can't run it again until you restart the Unreal Editor. | 2018-07-16 | [UNR-350](https://improbableio.atlassian.net/browse/UNR-350) | Ensure destination files/folders are unlocked. |
| Stably-named replicated actors cannot be referred to by their path | 2018-08-10 | [UNR-473](https://improbableio.atlassian.net/projects/UNR/issues/UNR-473) | None |
| Server travel does not work in PIE. | 2018-10-24 | | Server travel work must be done with external or managed workers |
| Server travel does not work in multi-worker configurations.  | 2018-10-24 | [UNR-678](https://improbableio.atlassian.net/projects/UNR/issues/UNR-678)
| `ReplicateYes` policy on GameplayAbilities not supported. | 2018-10-24 | [UNR-675](https://improbableio.atlassian.net/projects/UNR/issues/UNR-675) | Don't use replicated GameplayAbilities. If they need access to replicated data, store it on the AbilityComponent itself. |
| NetDeltaSerialize or Fast TArray Replication not supported.  | 2018-10-24 |  | Use default serialization |
| Non-replicated instanced data on stably-named replicated actors is reset to defaults on world startup. | 2018-10-29 | [UNR-624](https://improbableio.atlassian.net/projects/UNR/issues/UNR-624) | Mark the desired properties as replicated, or change them in a blueprint subclass. |
| Sometimes the player is unable to move when spawning | 2018-10-30 | [UNR-691](https://improbableio.atlassian.net/browse/UNR-691) | Reconnect| 
| Seamless Travel is not supported | 2019-01-22 | [UNR-897](https://improbableio.atlassian.net/browse/UNR-897) | Disable Seamless travel |  
| Gameplay Ability System is not fully supported | 2019-02-01 | | Use the workarounds detailed on the [Gameplay Ability System]({{urlRoot}}/content/ability-system) reference page. |  
| Opening the Editor while you are uploading or launching a cloud deployment disables the GDK Toolbar **Start** button, and Selecting **Stop** on the GDK Toolbar stops the running cloud deployment. | 2019-02-18 | [UNR-1006](https://improbableio.atlassian.net/browse/UNR-1006) | Wait for the cloud launch to finish, or launch the cloud deployment whilst the Editor is already open. |
| Blueprint RPCs with array arguments or "by reference" arguments are not serialized correctly when they are sent through SpatialOS. | 2019-02-18 | [UNR-1011](https://improbableio.atlassian.net/browse/UNR-1011) | Wrap any array arguments in a struct and change any "by reference" arguments to "by value" (default). |
