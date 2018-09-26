<%(Callout type="warn" message="This [pre-alpha](https://docs.improbable.io/reference/latest/shared/release-policy#maturity-stages) release of the SpatialOS GDK for Unreal is for evaluation and feedback purposes only, with limited documentation - see the guidance on [Recommended use]({{urlRoot}}/index#recommended-use)")%>

# Glossary

## SpatialOS terms
See the [SpatialOS glossary](https://docs.improbable.io/reference/latest/shared/glossary) for terms specific to SpatialOS.

## GDK for Unreal repositories
* The SpatialOS GDK for Unreal: https://github.com/spatialos/UnrealGDK
* Starter Project: https://github.com/spatialos/UnrealGDKStarterProject
* SpatialOS GDK for Unreal fork of Unreal Engine: https://github.com/improbableio/UnrealEngine/tree/4.19-SpatialOSUnrealGDK<br>
You may get a 404 error from this link. To get access, see [these instructions]({{urlRoot}}/setup-and-installing#unreal-engine-eula).
* Third-Person Shooter: https://github.com/spatialos/UnrealGDKThirdPersonShooter
* Test Suite: https://github.com/spatialos/UnrealGDKTestSuite

## GDK for Unreal terms

### Actor handover
 Handover is a new `UPROPERTY` tag. It allows games built in Unreal which uses single-server architecture to take advantage of SpatialOS’ distributed, persistent server architecture. See [Actor and entity property handover between server-workers]({{urlRoot}}/content/handover-between-server-workers.md).

### Global State Manager
The Global State Manager (GSM) makes sure that Singleton Actors are replicated properly, by only allowing the server-worker with [authority](https://docs.improbable.io/reference/latest/shared/glossary#read-and-write-access-authority) over the GSM to execute the initial replication of these Actors. See documentation on [Singleton Actors]({{urlRoot}}/content/singleton-actors.md).

### GSM
Short for "Global State Manager".

### Schema Generator
An Unreal Editor toolbar command which takes a set of Unreal classes and generates SpatialOS schema that enables automated communication between Unreal and SpatialOS. See the [SpatialOS website documentation’s .schema introduction](https://docs.improbable.io/reference/latest/shared/schema/introduction) for more information.

### Singleton Actor
A server-side authoritative Actor that is restricted to one instantiation on SpatialOS. See documentation on [Singleton Actors]({{urlRoot}}/content/singleton-actors.md).

[//]: # (Editorial review status: Full review 2018-07-23)
[//]: # (Issues to deal with, but not limited to:)
[//]: # (1. Adding more terms)
