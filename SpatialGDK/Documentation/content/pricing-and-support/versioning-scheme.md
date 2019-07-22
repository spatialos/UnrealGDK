<%(TOC)%>

# Versioning scheme

The SpatialOS GDK for Unreal consists of:

*   [The GDK Plugin](https://github.com/spatialos/unrealgdk)
*   [The SpatialOS Unreal Engine Fork](https://github.com/improbableio/UnrealEngine)
*   [The Example Project](https://github.com/spatialos/UnrealGDKExampleProject)

Each of these follows the same versioning scheme:

`MAJOR.MINOR.PATCH(-preview)`

* `MAJOR`: indicates the major version, currently 0 as the GDK is in Alpha. We do not provide any long term support and stability guarantees on any versions, and do not backport fixes. The major version will turn to 1 when we enter Beta. 
* `MINOR`: indicates the minor version. Released about once a month and containing significant features. Not backward compatible (unlike the [SemVer](https://semver.org/spec/v2.0.0.html) versioning scheme, where minor versions are backward compatible). 
* `PATCH`: indicates updates containing significant patch releases. Not backwards compatible (similarly, this is unlike [SemVer](https://semver.org/spec/v2.0.0.html))
* `(-preview)`: if present, indicates versions which are preview-only. These versions are less stable than others and do not contain full documentation. They are opt-in only with the `preview` branch described below. 

For example: 

*   [Version 0.5.0-preview](https://github.com/spatialos/UnrealGDK/releases/tag/0.5.0-preview) released on June 25th.
*   [Version 0.4.2](https://github.com/spatialos/UnrealGDK/releases/tag/0.4.2) released on May 20th.

## Which branches to use

When setting up the GDK for your project, you need to decide which branch of each of the repositories (listed above) to clone.

**The most stable version is always the `release` branch, which is set as the default branch for each repository.** We test this branch end-to-end with the SpatialOS SDKs, Tools, and Metagame Services, and it is fully documented. We update it a few times a year as part SpatialOS for Unreal milestones such as 2019.1.

An alternative option is to use the `preview` branch. This branch contains the latest `*-preview` version, which we test using Improbable partner projects, but not as extensively as versions in `release`. Preview releases are for teams already developing on the GDK and working closely with Improbable. 

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

### Unreal Engine Fork branches

The [Unreal Engine Fork](https://github.com/improbableio/UnrealEngine) follows the versioning and branches pattern described above, but the branch names have `X.XX-SpatialOSUnrealGDK` following prepended to them (where `X.XX` is the Unreal Engine version of the fork.)

For example:

*   ``4.22-SpatialOSUnrealGDK-release`` (default)
*   ``4.22-SpatialOSUnrealGDK-preview``

when Unreal Engine 4.22 is the version supported.

## Unreal Engine version support

**The GDK has full support for one version of Unreal Engine at a time** - the version that is shown in the Unreal Engine Fork repository’s default branch name, and is indicated in the Get Started documentation.
This is the only version that the Example Project supports. With version 0.6.0, this is Unreal Engine version 4.22. 

To enable you to transition, the GDK has legacy support during one release cycle for the Unreal Engine version previously supported. For example, with version 0.6.0, the legacy support is for Unreal Engine 4.20, which is available through the ``4.20-SpatialGDK-release`` branch of the SpatialOS Engine Fork. The Example Project is no longer supported with this version.

We endeavour to support the latest stable and recommended version of Unreal Engine less than 3 months after it's released. To see when a new version will be supported, check out the [Development Roadmap](https://github.com/spatialos/unrealgdk/projects/1).

<br/>------<br/>
_2019-07-21 Page added with limited editorial review_

[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1231)


