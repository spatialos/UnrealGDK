> This [pre-alpha](https://docs.improbable.io/reference/latest/shared/release-policy#maturity-stages) release of the SpatialOS Unreal GDK is for evaluation and feedback purposes only, with limited documentation - see the guidance on [Recommended use](../README.md#recommended-use).

# SpatialOS Unreal GDK documentation

## How to get started
We recommend you start by following the [Installation and setup guide](setup-and-installing.md) to set up the Unreal GDK, the SpatialOS Unreal Engine fork, and the Starter Project (or your own Unreal project).

After you've run through the setup guide (including setting up Actor replication), take a look at the following pages, which explain some of the key concepts to understand when developing SpatialOS games:

* [Interop Code Generator](./content/interop.md)
* [Actor handover](./content/handover-between-server-workers.md)

To get some background on the GDK, you can check out [our blogpost](https://improbable.io/games/blog/spatialos-unreal-gdk-pre-alpha).

We want your feedback on the Unreal GDK, its documentation and its [roadmap](https://trello.com/b/7wtbtwmL/unreal-gdk-roadmap) (Trello board) - [find out how to give us feedback](../README.md#give-us-feedback).

## Documentation

### SpatialOS
The Unreal GDK documentation assumes you are familiar with SpatialOS concepts. For guidance on SpatialOS concepts see the documentation on the [SpatialOS website](https://docs.improbable.io/reference/latest/shared/concepts/spatialos).

### Unreal GDK

#### Getting started
* [Installation and setup](setup-and-installing.md)
* [Features list](features.md)

#### Known issues
* [Known issues](known-issues.md)

#### Contents

Reference docs
* [Glossary](content/glossary.md)
* [Directory structure](content/directory-structure.md)
* [Features](./features.md)
* [Helper scripts](content/helper-scripts.md)
* [The SpatialOS Unreal GDK toolbar](content/toolbar.md)
* [Interop Code Generator (and type bindings)](content/interop.md)
* [Supported replicated types](content/supported-replicated-types.md)
* [Actor property handover between SpatialOS servers](content/handover-between-server-workers.md)
* [Singleton Actors (and the Global State Manager)](content/singleton-actors.md)
* [Troubleshooting](content/troubleshooting.md)

How-to docs
* [Porting a native Unreal project to the Unreal GDK](content/porting-unreal-project-to-gdk.md)
* [Generating a snapshot](./content/generating-a-snapshot.md)

#### Contributions
We are currently not accepting public contributions. However, we are accepting issues and we do want your [feedback](../README.md#give-us-feedback).
* [Contributions policy](../.github/CONTRIBUTING.md)
* [Coding standards](contributions/unreal-gdk-coding-standards.md)

&copy; 2018 Improbable