<%(TOC)%>
# Command line arguments

In the SpatialOS GDK for Unreal, you can use [command line arguments](https://docs.unrealengine.com/en-us/Programming/Basics/CommandLineArguments) to customize how the GDK runs. For example, you can specify how a worker instance connects to a SpatialOS deployment or choose which [network stack](https://docs.improbable.io/reference/13.7/shared/worker-configuration/network-configuration#choosing-a-network-stack) that a worker instance uses to communicate with the SpatialOS [Runtime](https://docs.improbable.io/reference/13.7/shared/glossary#the-runtime).

## Connection arguments

You can select one of the following ways to connect a worker instance to a SpatialOS deployment:

 * Using [Receptionist]({{urlRoot}}/content/map-travel#using-receptionist) to connect a client-worker or server-worker instance to a local or cloud deployment.
 * Using [Locator]({{urlRoot}}/content/map-travel#using-locator) to connect a client-worker instance to cloud deployment.

> **Note**: Use only one way for connection at a time; otherwise, errors might occur.

To learn more about the connection arguments that you can apply to both Receptionist and Locator, see the following table:

<table style="width: 100%;">
    <tbody>
        <tr>
            <td style="width: 27%;"><strong>Argument</strong></td>
            <td style="width: 10%;"><strong>Type</strong></td>
            <td style="width: 63%;"><strong>Description</strong></td>
        </tr>
        <tr>
            <td style="width: 27%;"><code>-workerId</code></td>
            <td style="width: 10%;">string</td>
            <td style="width: 63%;">The <a href="https://docs.improbable.io/reference/13.7/shared/glossary#worker-id">worker ID</a> that the worker instance uses to connect to a SpatialOS deployment. If no value is specified, a random <code>workerId</code> is created.</td>
        </tr>
        <tr>
            <td style="width: 27%;"><code>-workerType</code></td>
            <td style="width: 10%;">string</td>
            <td style="width: 63%;">The <a href="https://docs.improbable.io/reference/13.7/shared/glossary#worker-types-and-worker-instances">worker type</a> that the worker instance uses to connect to a SpatialOS deployment. In the GDK, you can specify the following worker types:
                <ul>
                    <li><code>UnrealWorker</code> for server-workers</li>
                    <li><code>UnrealClient</code> for client-workers</li>
                </ul>
                <p>For example, to connect a server-worker instance to a local deployment, you can run the following line:</p>
                <p><code>MyProject.exe MyMap -server -workerType UnrealWorker</code></p>
            </td>
        </tr>
        <tr>
            <td style="width: 27%;"><code>-useExternalIpForBridge</code></td>
            <td style="width: 10%;">bool</td>
            <td style="width: 63%;">Set <code>true</code> to connect a worker instance to a deployment in either of the following ways. By default, it‘s set to <code>false</code>.
                <ul>
                    <li>To connect to a deployment on a different machine (not hosted in the cloud by Improbable) from the worker instance’s machine.</li>
                    <li>To connect to a cloud deployment using <code>spatial cloud connect external</code>.</li>
                </ul>
            </td>
        </tr>
    </tbody>
</table>

### Receptionist arguments

When you connect a client-worker or server-worker instance to a SpatialOS depolyment without the [authentication flow](https://docs.improbable.io/reference/13.7/shared/auth/integrate-authentication-platform-sdk#authentication-flow), add the following Receptionist arguments before you launch the deployment:

<table style="width: 100%;">
    <tbody>
        <tr>
            <td style="width: 27%;"><strong>Argument</strong></td>
            <td style="width: 10%;"><strong>Type</strong></td>
            <td style="width: 63%;"><strong>Description</strong></td>
        </tr>
        <tr>
            <td style="width: 27%;"><code>-receptionistHost</code><br></td>
            <td style="width: 10%;">string</td>
            <td style="width: 63%;">The IP of the deployment that the worker instance connects to. By default, it's <code>127.0.0.1</code>.</td>
        </tr>
        <tr>
            <td style="width: 27%;"><code>-receptionistPort</code><br></td>
            <td style="width: 10%;">string</td>
            <td style="width: 63%;">The port of the deployment that the worker instance connects to. By default, it's <code>7777</code>.</td>
        </tr>
    </tbody>
</table>

For example, to connect a server-worker instance to a deployment on a different machine using Receptionist, you can run the following line:

`MyProject.exe MyMap -server -workerType UnrealWorker -receptionistHost <IP of a different machine> -useExternalIpForBridge true`

### Locator arguments

When you use the [SpatialOS Launcher](https://docs.improbable.io/reference/13.7/shared/operate/launcher#the-launcher) or [player authentication](https://docs.improbable.io/reference/13.7/platform-sdk/scenarios/player-auth#player-authentication) to connect a client-worker instance to a SpatialOS depolyment, add the following Locator arguments before you launch the deployment:

<table style="width: 100%;">
    <tbody>
        <tr>
            <td style="width: 27%;"><strong>Argument</strong></td>
            <td style="width: 10%;"><strong>Type</strong></td>
            <td style="width: 63%;"><strong>Description</strong></td>
        </tr>
        <tr>
            <td style="width: 27%;"><code>-locatorHost</code><br></td>
            <td style="width: 10%;">string</td>
            <td style="width: 63%;">The IP of the Locator service that the client-worker instance connects to. You're recommended to use the default value, which is <code>locator.improbable.io</code>.<br></td>
        </tr>
        <tr>
            <td style="width: 27%;"><code>-playerIdentityToken</code><br></td>
            <td style="width: 12;">string</td>
            <td style="width: 63%;">The <code><a href="https://docs.improbable.io/reference/13.7/shared/auth/integrate-authentication-platform-sdk#2-generating-a-playeridentitytoken">PlayerIdentifyToken</a></code> (PIT) that the client-worker instance passes to the Locator service to prove
                its player identity within SpatialOS.<br></td>
        </tr>
        <tr>
            <td style="width: 27%;"><code>-loginToken</code><br></td>
            <td style="width: 10%;">string</td>
            <td style="width: 63%;">
                <p dir="ltr">The <code><a href="https://docs.improbable.io/reference/13.7/shared/auth/integrate-authentication-platform-sdk#3-generating-a-logintoken">LoginToken</a></code> that the client-worker instance passes to the Locator service to be permitted to join the
                    deployment.</p>
            </td>
        </tr>
    </tbody>
</table>

## Other arguments

Use the following arguments to configure settings other than worker connection:

<table style="width: 100%;">
    <tbody>
        <tr>
            <td style="width: 27%;"><strong>A</strong><strong>rgument</strong></td>
            <td style="width: 10%;"><strong>Type</strong></td>
            <td style="width: 63%;"><strong>Description</strong></td>
        </tr>
        <tr>
            <td style="width: 27%;"><code>-NoLogToSpatial</code><br></td>
            <td style="width: 10%;">flag</td>
            <td style="width: 63%;">To stop the server-worker instance from sending warning or error log messages to SpatialOS, add this flag. &nbsp;<br><br><strong>Note</strong>: You can add this flag to help improve SpatialOS performance when a large number of warning or error
                log messages are generated.</td>
        </tr>
        <tr>
            <td style="width: 27%;"><code>-useQBI</code></td>
            <td style="width: 10%;">bool</td>
            <td style="width: 63%;">Set <code>true</code> to enable query-based interest. By default, it's set to <code>false</code>.<br></td>
        </tr>
        <tr>
            <td style="width: 27%;"><code>-enableProtocolLogging</code><br></td>
            <td style="width: 10%;">bool</td>
            <td style="width: 63%;">Set <code>true</code> to enable [protocol logging](add a link).<br></td>
        </tr>
        <tr>
            <td style="width: 27%;"><code>-protocolLoggingPrefix</code><br></td>
            <td style="width: 10%;">string</td>
            <td style="width: 63%;">The path that the protocol logs are generated to. <br><br><strong>Note</strong>: If you enable protocol logging, you must set this value.</td>
        </tr>
        <tr>
            <td style="width: 27%;"><code>-linkProtocol</code><br></td>
            <td style="width: 10%;">string</td>
            <td style="width: 63%;">The network stack that the worker instance uses to communicate with the Runtime.
                The available options are <a href="https://docs.improbable.io/reference/13.7/shared/worker-configuration/network-configuration#kcp">KCP</a> and <a href="https://docs.improbable.io/reference/13.7/shared/worker-configuration/network-configuration#tcp">TCP</a>. The default value is <code>KCP</code>.<br><br><strong>Note</strong>:
                The <a href="https://docs.improbable.io/reference/13.7/shared/worker-configuration/network-configuration#raknet">RakNet</a> network stack is not supported.<br></td>
        </tr>
    </tbody>
</table>

## URL options

In the GDK, so that a client-worker instance executes a [map travel]({{urlRoot}}/content/map-travel), use `ClientTravel` and the Locator flow to change which cloud deployment that the client-worker instance connects to. Add the following [URL options](https://docs.unrealengine.com/en-us/Programming/Basics/CommandLineArguments) to the `ClientTravel` URL:

<table style="width: 100%;">
    <tbody>
        <tr>
            <td style="width: 27%;"><strong>Option</strong></td>
            <td style="width: 10%;"><strong>Type</strong></td>
            <td style="width: 63%;"><strong>Description</strong></td>
        </tr>
        <tr>
            <td style="width: 27%;"><code>locator</code></td>
            <td style="width: 10%;">flag</td>
            <td style="width: 63%;">To connect the client-worker instance to the deployment using Locator, &nbsp;add this flag to the <code>ClientTravel</code> URL.<br><br><strong>Note</strong>: If you add this flag, ensure that the <code>playeridentity</code> option and the
                <code>login</code> option are also in the URL.</td>
        </tr>
        <tr>
            <td style="width: 27%;"><code>playeridentity</code></td>
            <td style="width: 10%;">string</td>
            <td style="width: 63%;">The <code><a href="https://docs.improbable.io/reference/13.7/shared/auth/integrate-authentication-platform-sdk#2-generating-a-playeridentitytoken">PlayerIdentifyToken</a></code> (PIT) that the client-worker instance passes to the Locator service
                to prove its player identity within SpatialOS.</td>
        </tr>
        <tr>
            <td style="width: 27%;"><code>login</code></td>
            <td style="width: 10%;">string</td>
            <td style="width: 63%;">The <code><a href="https://docs.improbable.io/reference/13.7/shared/auth/integrate-authentication-platform-sdk#3-generating-a-logintoken">LoginToken</a></code> that the client-worker instance passes to the Locator service to be permitted to
                join the deployment.</td>
        </tr>
        <tr>
            <td style="width: 27%;"><code>useExternalIpForBridge</code></td>
            <td style="width: 10%;">bool</td>
            <td style="width: 63%;">Set <code>true</code> to connect a client-worker instance to a deployment in either of the following ways. By default, it's set to <code>false</code>.</code>
                <ul>
                    <li>To connect to a deployment on a different machine (not hosted in the cloud by Improbable) from the worker instance&#39;s machine.</li>
                    <li>To connect to a cloud deployment using <code>spatial cloud connect external</code>.</li>
                </ul>
            </td>
        </tr>
    </tbody>
</table>