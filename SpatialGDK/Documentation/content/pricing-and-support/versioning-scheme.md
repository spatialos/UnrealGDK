<%(TOC)%>

# Versioning scheme

The SpatialOS GDK for Unreal consists of:

*   [The SpatialOS Unreal Engine fork](https://github.com/improbableio/UnrealEngine)
*   [The GDK plugin](https://github.com/spatialos/unrealgdk) (includes the Starter Template, a blank starter project)
*   [The Example Project](https://github.com/spatialos/UnrealGDKExampleProject)

These repositories all follow the same versioning scheme, as described below: 

`MAJOR.MINOR.PATCH(-preview)`

* `MAJOR`: indicates the major version, currently 0 as the GDK is in Alpha. We do not provide any long term support and stability guarantees on any versions, and do not backport fixes. The major version will turn to 1 when we enter Beta. 
* `MINOR`: indicates the minor version. Released about once a month and containing significant features. Not backward compatible while the GDK is in Alpha.
* `PATCH`: indicates updates containing significant patch releases. Not backwards compatible while the GDK is in Alpha.
* `(-preview)`: if present, indicates versions which are preview-only. These versions are less stable than others and do not contain full documentation. They are opt-in and they live on the `preview` branch, as opposed to `release`.

For example: 

*   [Version 0.5.0-preview](https://github.com/spatialos/UnrealGDK/releases/tag/0.5.0-preview) released on June 25th.
*   [Version 0.4.2](https://github.com/spatialos/UnrealGDK/releases/tag/0.4.2) released on May 20th.

## Which branches to use

When setting up the GDK for your project, you need to decide which branch of each of the repositories (listed above) to clone.

**The most stable version is always the `release` branch, which is set as the default branch for each repository.** We test this branch end-to-end with the SpatialOS SDKs, Tools, and Metagame Services, and it is fully documented. We update it a few times a year as part SpatialOS for Unreal milestones such as 2019.1.

An alternative option is to use the `preview` branch. This branch contains the latest `*-preview` version, which we test using selected customer projects, but not as extensively as versions in `release`. Preview releases are for teams already developing on the GDK and working closely with Improbable. 

In summary:

<table>
  <tr>
   <td><strong>Branch</strong>
   </td>
   <td><strong>Update frequency</strong>
   </td>
   <td><strong>Maturity</strong>
   </td>
   <td><strong>Support policy</strong>
   </td>
  </tr>
  <tr>
   <td><code>release</code>
   </td>
   <td>A few times per year (as part of SpatialOS for Unreal milestones such as 2019.1).
   </td>
   <td>Fully tested, stable, and documented.
   </td>
   <td>Fully supported via our forums and Discord.
   </td>
  </tr>
  <tr>
   <td><code>preview</code>
   </td>
   <td>~Monthly
   </td>
   <td>Minimal tested focused on partner projects. Documentation is incomplete.
   </td>
   <td>Support is only provided to customers on the paid support tier.
   </td>
  </tr>
</table>

### Unreal Engine fork branches

The [Unreal Engine fork](https://github.com/improbableio/UnrealEngine) follows the versioning and branches pattern described above, but the branch names have `X.XX-SpatialOSUnrealGDK` following prepended to them (where `X.XX` is the Unreal Engine version of the fork.)

For example:

*   ``4.23-SpatialOSUnrealGDK-release`` (default)
*   ``4.23-SpatialOSUnrealGDK-preview``

when Unreal Engine 4.23 is the version supported.

## Unreal Engine version support

**The GDK and the Example Project only support one version of Unreal Engine at a time**. This version is the Unreal Engine fork repository’s default branch, and is indicated in the [Get started]({{urlRoot}}/content/get-started/introduction) documentation.

To facilitate a smooth upgrade for your project, when we introduce support for a new Unreal Engine version, we also support the previously Engine version in parallel, for one GDK release. For example, GDK version `0.6.0` introduces `Unreal Engine 4.22` support, but is also the last GDK version to support `4.20-SpatialOSUnrealGDK-release`.

We endeavour to support the latest stable and recommended version of Unreal Engine less than 3 months after it's released. To see when a new version will be supported, check out the [Development Roadmap](https://github.com/spatialos/unrealgdk/projects/1).

<br/>------<br/>
_2019-08-07 Page updated: clarified what the GDK consists of_
<br>_2019-07-21 Page added with limited editorial review_

[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1231)

