#### Description
Describe your changes here.

#### Release note
REQUIRED: Add a release note to the `##Unreleased` section of CHANGELOG.md. You can find guidance for writing useful release notes [here](../SpatialGDK/Extras/internal-documentation/how-to-write-good-release-notes.md). Documentation changes are exempt from this requirement.

#### Tests
How did you test these changes prior to submitting this pull request?
What automated tests are included in this PR?

STRONGLY SUGGESTED: How can this be verified by QA?

#### Documentation
How is this documented (for example: release note, upgrade guide, feature page, in-code documentation)?

#### Reminders (IMPORTANT)
If your change relies on a breaking engine change:
* Increment `SPATIAL_ENGINE_VERSION` in `Engine\Source\Runtime\Launch\Resources\SpatialVersion.h` (in the engine fork) as well as `SPATIAL_GDK_VERSION` in `SpatialGDK\Source\SpatialGDK\Public\Utils\EngineVersionCheck.h`. This helps others by providing a more helpful message during compilation to make sure the GDK and the Engine are up to date.

If your change updates `Setup.bat`, `Setup.sh`, core SDK version, any C# tools in `SpatialGDK\Build\Programs\Improbable.Unreal.Scripts`, or hand-written schema in `SpatialGDK\Extras\schema`:
* Increment the number in `RequireSetup`. This will automatically run `Setup.bat` or `Setup.sh` when the GDK is next pulled.

#### Primary reviewers
If your change will take a long time to review, you can name at most two primary reviewers who are ultimately responsible for reviewing this request. @ mention them.
