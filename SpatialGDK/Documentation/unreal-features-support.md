# Unreal features support 

The aim of the GDK is to seamlessly support all native Unreal Engine features, making it easy to create and port any Unreal game code to run on SpatialOS, both in single server and multiserver configurations.

The following tables show the state of support of Unreal Engine features on the GDK, along with any caveats or workaround you should be aware of. The Unreal Engine features support for multiserver configurations of the GDK will be available in Q3 2019.

<style type="text/css">
    th {
        vertical-align:middle;
    }

    td {
        vertical-align:middle;
    }

    .supported {
        background-color: #3BE2A0;
    }

    .caveats {
        background-color:#597CF4;
    }

    .indev {
        background-color:#FFD058;
    }

    .roadmap {
        background-color:#FF6187;
    }

    /* .na {
        background-color:#808080;
    } */
</style>

**Legend**

<table >
    <tr>
        <td>Fully supported, available now</td>
        <td class="supported"></td>
    <tr>
        <td>Supported with caveats or workarounds</td>
        <td class="caveats"></td>
    </tr>
        <td>Q2 - Q3 2019</td>
        <td class="indev"></td>
    </tr>
        <td>Planned for post Q3 2019</td>
        <td class="roadmap"></td>
    </tr>
    <!-- </tr>
        <td>Not applicable, or not planned to be delivered</td>
        <td class="na"></td>
    </tr> -->
</table>

## Single-Server Support

Support of Unreal features with the GDK in a single server-worker configuration:

<table style="width:100%">
  <tr style="background-color:#f0f0f0;">
    <th>Feature Area (Links are to Unreal documentation)</th>
    <th style="width:25%">Feature (Links are to Unreal documentation)</th>
    <th style="width:2%;">Support Level</th>
    <th>Notes & Caveats</th>
  </tr>
  
  <!-- ------ Gameplay Framework ------ -->

  <tr>
    <td rowspan="7"><a href="https://docs.unrealengine.com/en-us/Gameplay/Framework"><b>Gameplay Framework<b></a></td>
    <td>GameMode</td>
    <td class="supported"></td>
    <td></td>
  </tr>
  <tr>
    <td>GameState</td>
    <td class="supported"></td>
    <td></td>
  </tr>
  <tr>
    <td>PlayerState</td>
    <td class="supported"></td>
    <td></td>
  </tr>
  <tr>
    <td>PlayerController</td>
   <td class="supported"></td>
    <td></td>
  </tr>
  <tr>
    <td>Character / Pawn</td>
    <td class="supported"></td>
    <td></td>
  </tr>
  <tr>
    <td>Level Blueprints</td>
    <td class="supported"></td>
    <td></td>
  </tr>
  <tr>
    <td>Ability System</td>
    <td class="caveats"></td>
    <td>Supported with workarounds described <a href="{{urlRoot}}/content/ability-system">here</a>. Replicated ability components not supported.</td>
  </tr>

  <!-- ------ Property Replication ------ -->
  
   <tr>
    <td rowspan="9"><a href="https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors/Properties"><b>Property Replication<b></a></td>
    <td>Data Property Replication (C++ classes)</td>
    <td class="supported"></td>
    <td>Using fixed arrays instead of TArrays is significantly faster.</td>
  </tr>
  <tr>
    <td>Data Property Replication (Blueprint classes)</td>
    <td class="supported"></td>
    <td></td>
  </tr>
  <tr>
    <td>Dynamic Object Reference Replication (C++ classes)</td>
    <td class="supported"></td>
    <td></td>
  </tr>
  <tr>
    <td>Dynamic Object Reference Replication (Blueprint classes)</td>
   <td class="supported"></td>
    <td></td>
  </tr>
  <tr>
    <td>Stably Named Object Reference Replication</td>
    <td class="supported"></td>
    <td></td>
  </tr>
  <tr>
    <td>Conditional Replication</td>
    <td class="supported"></td>
    <td>Replication condition active overrides (DOREPLIFETIME_ACTIVE_OVERRIDE as defined  <a href="https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors/Properties/Conditions">in Unreal documentation</a>) do not work.</td>
  </tr>
  <tr>
    <td>Delta Serialization</td>
    <td class="roadmap"></td>
    <td></td>
  </tr>
    <tr>
    <td>Fast TArray Serialization</td>
    <td class="roadmap"></td>
    <td>Supported, but slower than native UE.</td>
  </tr>
  <tr>
    <td>RepNotify Callbacks</td>
    <td class="supported"></td>
    <td></td>
  </tr>

<!-- ------ Actor Replication ------ -->
  
   <tr>
    <td rowspan="7"><b><a href="https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors">Actor Replication</a><b></td>
    <td>Client and server RPCs (C++)</td>
    <td class="supported"></td>
    <td>Ordering of reliable RPCs is not respected.</td>
  </tr>
  <tr>
    <td>Client and Server RPCs (Blueprint)</td>
    <td class="supported"></td>
    <td>Ordering of reliable RPCs is not respected.</td>
  </tr>
  <tr>
    <td>Multicast RPCs</td>
    <td class="supported"></td>
    <td>RPCs cannot be reliable. (This is due to the distributed systems nature of SpatialOS.)</td>
  </tr>
  <tr>
    <td>Actor Roles</td>
   <td class="supported"></td>
    <td></td>
  </tr>
  <tr>
    <td>Static Subobject Replication</td>
    <td class="supported"></td>
    <td></td>
  </tr>
  <tr>
    <td>Dynamic Component Replication</td>
    <td class="indev"></td>
    <td></td>
  </tr>
  <tr>
    <td>Startup Actors</td>
    <td class="supported"></td>
    <td></td>

<!-- ------ Multiplayer Gameplay Features ------ -->

 <tr>
    <td rowspan="7"><b>Multiplayer Gameplay Features<b></td>
    <td><a href="https://docs.unrealengine.com/en-us/Engine/Rendering/ParticleSystems/Optimization/SplitScreen">Split Screen</a></td>
    <td class="roadmap"></td>
    <td></td>
  </tr>
  <tr>
    <td><a href="https://docs.unrealengine.com/en-us/Engine/Replay">Replay System</a></td>
    <td class="roadmap"></td>
    <td></td>
  </tr>
  <tr>
    <td><a href="https://docs.unrealengine.com/en-US/Gameplay/Networking/CharacterMovementComponent">Character Movement</a></td>
    <td class="supported"></td>
    <td></td>
  </tr>
  <tr>
    <td><a href="https://docs.unrealengine.com/en-us/Engine/Physics/Vehicles">Vehicle Movement</a></td>
   <td class="supported"></td>
    <td></td>
  </tr>
  <tr>
    <td><a href="https://docs.unrealengine.com/en-us/Gameplay/Networking/OnlineBeacons">Online Beacons</a></td>
    <td class="roadmap"></td>
    <td></td>
  </tr>
  <tr>
    <td><a href="https://docs.unrealengine.com/en-us/Programming/UnrealArchitecture/Timers">Timers</a></td>
    <td class="supported"></td>
    <td></td>
  </tr>
  <tr>
    <td><a href="https://docs.unrealengine.com/en-us/Programming/Online">Online Subsystem Abstraction</a></td>
    <td class="indev"></td>
    <td></td>

<!-- ------ Optimization ------ -->

 <tr>
    <td rowspan="3"><b>Optimization<b></td>
    <td><a href="https://docs.unrealengine.com/en-us/Engine/Networking/ReplicationGraph">Replication Graph</a></td>
    <td class="indev"></td>
    <td>We will present a different system for the same purpose instead.</td>
  </tr>
  <tr>
    <td><a href="https://docs.unrealengine.com/en-US/Gameplay/Networking/Actors/ReplicationFlow">Net Dormancy</a></td>
    <td class="roadmap"></td>
    <td></td>
  </tr>
  <tr>
    <td><a href="https://docs.unrealengine.com/en-US/Gameplay/Networking/Actors/ReplicationFlow">Net Update Frequency</a></td>
    <td class="supported"></td>
    <td></td>
  </tr>

<!-- ------ Debugging ------ -->

 <tr>
    <td rowspan="3"><b>Debugging<b></td>
    <td><a href="https://docs.unrealengine.com/en-us/Gameplay/Tools/GameplayDebugger">Gameplay Debugger</a></td>
    <td class="indev"></td>
    <td>Currently unsupported due to NetDeltaSerialize dependency.</td>
  </tr>
  <tr>
    <td><a href="https://api.unrealengine.com/INT/BlueprintAPI/CheatManager/index.html">Cheat Manager</a></td>
    <td class="supported"></td>
    <td></td>
  </tr>
  <tr>
    <td><a href="https://docs.unrealengine.com/en-us/Gameplay/Tools/NetworkProfiler">Network Profiler</td>
    <td class="roadmap"></td>
    <td>We may not support this tool fully but will have equivalent functionality in <a href="https://docs.improbable.io/reference/13.7/shared/operate/metrics"> SpatialOS metrics</a>.</td>
  </tr>

<!-- ------ World Building ------ -->

 <tr>
    <td rowspan="2"><a href="https://docs.unrealengine.com/en-us/Engine/LevelStreaming/WorldBrowser"><b>World Building<b></a></td>
    <td>World Composition / Streaming (Level Streaming)</td>
    <td class="supported"></td>
    <td></td>
  </tr>
  <tr>
    <td>World Origin Shifting</td>
    <td class="roadmap"></td>
    <td></td>
</tr>

<!-- ------ AI ------ -->

 <tr>
    <td rowspan="3"><a href="https://docs.unrealengine.com/en-us/Gameplay/AI"><b>AI<b></td>
    <td>Behavior Trees</td>
    <td class="supported"></td>
    <td></td>
  </tr>
  <tr>
    <td>Blackboard</td>
    <td class="supported"></td>
    <td></td>
</tr>
  <tr>
    <td>Environment Query System</td>
    <td class="supported"></td>
    <td></td>
</tr>

<!-- ------ Travel ------ -->

 <tr>
    <td rowspan="3"><a href="https://docs.unrealengine.com/en-us/Gameplay/Networking/Travelling"><b>Travel<b></a></td>
    <td>Server Travel</td>
    <td class="roadmap"></td>
    <td></td>
  </tr>
  <tr>
    <td>Client Travel</td>
    <td class="supported"></td>
    <td></td>
</tr>
  <tr>
    <td>Seamless Travel</td>
    <td class="roadmap"></td>
    <td></td>
</tr>

<!-- ------ Misc ------ -->

 <tr>
    <td rowspan="5"><b>Misc<b></td>
    <td><a href="https://docs.unrealengine.com/en-us/Engine/Physics">Physics Simulation</a></td>
    <td class="supported"></td>
    <td></td>
  </tr>
  <tr>
    <td><a href="https://docs.unrealengine.com/en-us/Engine/Physics/Tracing">Tracing</a></td>
    <td class="supported"></td>
    <td></td>
</tr>
  <tr>
    <td><a href="https://docs.unrealengine.com/en-us/Engine/Animation">Skeletal Mesh Animation System</a></td>
    <td class="supported"></td>
    <td></td>
</tr>
  <tr>
    <td><a href="https://docs.unrealengine.com/Engine/Sequencer">Sequence Editor</td>
    <td class="supported"></td>
    <td></td>
</tr>
  <tr>
    <td>UE4 4.22 Support</td>
    <td class="indev"></td>
    <td>Expected before the end of June 2019.</td>
</tr>

<!-- ------ Platforms ------ -->

 <tr>
    <td rowspan="5"><b>Platforms<b></td>
    <td>PC</td>
    <td class="supported"></td>
    <td></td>
  </tr>
  <tr>
    <td>Xbox One</td>
    <td class="supported"></td>
    <td>Please see annoucement <a href="https://improbable.io/blog/spatialos-now-supports-xbox-one-and-playstation-4-development">here</a>.</td>
</tr>
  <tr>
    <td>PS4</td>
    <td class="supported"></td>
    <td>Please see annoucement <a href="https://improbable.io/blog/spatialos-now-supports-xbox-one-and-playstation-4-development">here</a>.</td>
</tr>
  <tr>
    <td>Mobile</td>
    <td class="indev"></td>
    <td></td>
</tr>

</table>

## Multiserver Support

The table for multiserver support is coming soon. 

<br/>

------------
*2019-04-25 Page added with full editorial review*
<br/>
