<%(Callout type="warn" message="This [pre-alpha](https://docs.improbable.io/reference/latest/shared/release-policy#maturity-stages) release of the SpatialOS GDK for Unreal is for evaluation and feedback purposes only, with limited documentation - see the guidance on [Recommended use]({{urlRoot}}/index#recommended-use)")%>

# GDK for Unreal directory structure
The table below lists the contents of the Unreal GDK repository after running `Setup.bat`.

| Directory | Purpose
|-----------|---------
| `SpatialGDK/Binaries/ThirdParty/Improbable/` | Not tracked in git. This directory contains the required binaries for your Unreal project to work with the SpatialOS UnrealGDK. These files are generated when running `Setup.bat`.
| `SpatialGDK/Build/core_sdk/` | Not tracked in git. Contains the [C API worker SDK](https://docs.improbable.io/reference/latest/capi/introduction) dependencies used by the Unreal GDK to serialize data to and from SpatialOS.
| `SpatialGDK/Build/Programs/` | Contains the source and project files for the executables used when building the GDK.
| `SpatialGDK/Build/Scripts/` | Contains the helper scripts that allow you to build either a server-worker or a client-worker.
| `SpatialGDK/Documentation` | Contains the documentation for the GDK.
| `SpatialGDK/Extras/fastbuild` | Contains files related to [FASTBuild](http://www.fastbuild.org/docs/home.html), an open-source build system that is currently only usable with the GDK by Improbable engineers.
| `SpatialGDK/Extras/schema` | Contains the [schema files](https://docs.improbable.io/reference/latest/shared/glossary#schema) required for the GDK to interact with SpatialOS.
| `SpatialGDK/Extras/linting/` | Contains the scripts we use to lint the GDK.
| `SpatialGDK/Source/SpatialGDK/Public/WorkerSdk/` | Not tracked in git. Contains the [C API worker SDK](https://docs.improbable.io/reference/latest/capi/introduction) headers which are used while building the GDK. You install these when you run `Setup.bat`
| `SpatialGDK/Source/SpatialGDK/Public` | Contains the public source code of the GDK uplugin.
| `SpatialGDK/Source/SpatialGDK/Private` | Contains the private source code of the GDK uplugin.
| `/SpatialGDKEditorToolbar/` | Contains the [SpatialOS GDK toolbar]({{urlRoot}}/content/toolbar.md) that appears within the Unreal Editor GUI, from which you can take snapshots, generate schemas, start and stop deployments, and access the SpatialOS Inspector.
