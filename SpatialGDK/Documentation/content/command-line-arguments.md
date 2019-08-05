<%(TOC)%>
# Command line arguments
In the SpatialOS GDK for Unreal, you can use GDK-specific command line arguments to customize how the GDK runs. For example, you can specify how a worker instance connects to a [deployment]({{urlRoot}}/content/glossary#deployment) or choose which [network stack](https://docs.improbable.io/reference/latest/shared/worker-configuration/network-configuration#choosing-a-network-stack) a worker instance uses to communicate with the SpatialOS [Runtime]({{urlRoot}}/content/glossary#spatialos-runtime).

## Connection arguments
You can use one of the following methods to connect a worker instance to a SpatialOS deployment:

* Use the [Receptionist]({{urlRoot}}/content/map-travel#using-receptionist-local-deployments) to connect a client-worker or server-worker instance to a local or cloud deployment.
* Use the [Locator](https://docs.improbable.io/reference/latest/shared/glossary#locator) to connect a client-worker instance to a cloud deployment.

Each time you connect one worker instance to a deployment, you can choose only one method for connection. You might need to add both general [connection arguments](#connection-arguments) and method-specific arguments.

> **Note**: If you specify both Receptionist arguments and Locator arguments to connect one worker instance to a deployment, errors might occur.

When you use either method to connect a worker instance to a deployment, you can add the following general connection arguments:

<table>
    <tbody>
        <tr>
            <td style="width: 30%;"><strong>Argument</strong></td>
            <td style="width: 8%;"><strong>Type</strong></td>
            <td style="width: 62%;"><strong>Description</strong></td>
        </tr>
        <tr>
            <td style="width: 30%;"><code>-workerId</code></td>
            <td style="width: 8%;">string</td>
            <td style="width: 62%;">
                <p><span style="font-weight: 400;">The&nbsp;</span><a href="https://docs.improbable.io/reference/13.7/shared/glossary#worker-id"><span style="font-weight: 400;">worker ID</span></a><span style="font-weight: 400;">&nbsp;that the worker instance uses to connect to a SpatialOS deployment.&nbsp;</span></p>
                <p><span style="font-weight: 400;">If you don&rsquo;t specify a value, the worker instance generates a random worker ID and uses it to connect to a deployment.</span></p>
            </td>
        </tr>
        <tr>
            <td style="width: 30%;"><code>-workerType</code></td>
            <td style="width: 8%;">string</td>
            <td style="width: 62%;">
                <p><span style="font-weight: 400;">The&nbsp;</span><a href="https://docs.improbable.io/reference/13.7/shared/glossary#worker-types-and-worker-instances"><span style="font-weight: 400;">worker type</span></a><span style="font-weight: 400;">&nbsp;that the worker instance uses to connect to a SpatialOS deployment. In the GDK, you can specify the following worker types:</span></p>
                <ul>
                    <li><code>UnrealWorker</code> for server-workers</li>
                    <li><code>UnrealClient</code> for client-workers</li>
                </ul>
                <p>For example, to connect a server-worker instance to a local deployment, run:</p>
                <p><span style="font-weight: 400;"><code>MyProject.exe MyMap -server -workerType UnrealWorker</code></span></p>
                <p>Here are the default settings when you launch a deployment without specifiying any <code>workerType</code>:</p>
                <ul>
                    <li><span style="font-weight: 400;">When you run the command with the <code>-server</code> argument, you connect a server-worker instance to the deployment.</span></li>
                    <li><span style="font-weight: 400;">When you run the command without the <code>-server</code> argument, you connect a client-worker instance to the deployment.</span></li>
                </ul>
            </td>
        </tr>
        <tr>
            <td style="width: 30%;"><span style="font-weight: 400;"><code>-useExternalIpForBridge</code></span></td>
            <td style="width: 8%;">bool</td>
            <td style="width: 62%;">
                <p><span style="font-weight: 400;">Set to <code>true</code> to connect a worker instance to a deployment in either of the following ways:</span></p>
                <ul>
                    <li style="font-weight: 400;"><span style="font-weight: 400;">To connect to a deployment on a different machine from the worker instance&rsquo;s machine (not hosted in the cloud).</span></li>
                    <li><span style="font-weight: 400;">To connect to a cloud deployment using <code>spatial cloud connect external</code>.</span></li>
                </ul>
                <p>Default: <code>false</code></p>
            </td>
        </tr>
        <tr>
            <td style="width: 30%;"><code>-linkProtocol</code></td>
            <td style="width: 8%;">string</td>
            <td style="width: 62%;">The <a href="https://docs.improbable.io/reference/13.7/shared/worker-configuration/network-configuration#choosing-a-network-stack">network stack</a> that the worker instance uses to communicate with the SpatialOS <a href="https://docs.improbable.io/reference/13.7/shared/glossary#the-runtime">Runtime</a>.
                The available options are <a href="https://docs.improbable.io/reference/13.7/shared/worker-configuration/network-configuration#kcp">KCP</a> and <a href="https://docs.improbable.io/reference/13.7/shared/worker-configuration/network-configuration#tcp">TCP</a>.&nbsp;
                <p><strong>Note</strong>: The GDK does not support the <a href="https://docs.improbable.io/reference/13.7/shared/worker-configuration/network-configuration#raknet">RakNet</a> network stack.</p>
                <p>Default: <code>KCP</code></p>
            </td>
        </tr>
    </tbody>
</table>

### Receptionist arguments
When you connect a client-worker or server-worker instance to a deployment without the [authentication flow](https://docs.improbable.io/reference/13.7/shared/auth/integrate-authentication-platform-sdk#authentication-flow), you can add the following Receptionist arguments before you launch the deployment:

<table>
    <tbody>
        <tr>
            <td style="width: 30%;"><strong>Argument</strong></td>
            <td style="width: 8%;"><strong>Type</strong></td>
            <td style="width: 62%;"><strong>Description</strong></td>
        </tr>
        <tr>
            <td style="width: 30%;"><code>-receptionistHost</code></td>
            <td style="width: 8%;">string</td>
            <td style="width: 62%;">The IP of the deployment that the worker instance connects to.
                <p>Default: <code>127.0.0.1</code></p>
            </td>
        </tr>
        <tr>
            <td style="width: 30%;"><code>-receptionistPort</code></td>
            <td style="width: 8%;">string</td>
            <td style="width: 62%;">The port of the deployment that the worker instance connects to.
                <p>Default: <code>7777</code></p>
            </td>
        </tr>
    </tbody>
</table>

For example, to connect a server-worker instance to a deployment on a different machine using Receptionist, run:

`MyProject.exe MyMap -server -workerType UnrealWorker -receptionistHost <IP of a different machine> -useExternalIpForBridge true`

### Locator arguments
When you use the [SpatialOS Launcher](https://docs.improbable.io/reference/13.7/shared/operate/launcher#the-launcher) or [player authentication](https://docs.improbable.io/reference/13.7/platform-sdk/scenarios/player-auth#player-authentication) service to connect a client-worker instance to a deployment, add the following Locator arguments before you launch the deployment:

<table>
    <tbody>
        <tr>
            <td style="width: 30%;"><strong>Argument</strong></td>
            <td style="width: 8%;"><strong>Type</strong></td>
            <td style="width: 62%;"><strong>Description</strong></td>
        </tr>
        <tr>
            <td style="width: 30%;"><code>-playerIdentityToken</code></td>
            <td style="width: 8%;">string</td>
            <td style="width: 62%;">The <a href="https://docs.improbable.io/reference/13.7/shared/auth/integrate-authentication-platform-sdk#2-generating-a-playeridentitytoken"><span style="font-weight: 400;"><code>PlayerIdentityToken</code></span></a><span style="font-weight: 400;">&nbsp;(PIT) that the client-worker instance passes to the Locator service to prove its player identity within SpatialOS.</span></td>
        </tr>
        <tr>
            <td style="width: 30%;"><code>-loginToken</code></td>
            <td style="width: 8%;">string</td>
            <td style="width: 62%;">The <a href="https://docs.improbable.io/reference/13.7/shared/auth/integrate-authentication-platform-sdk#3-generating-a-logintoken"><code>LoginToken</code></a> that the client-worker instance passes to the Locator service to be permitted to
                join the deployment.</td>
        </tr>
    </tbody>
</table>

## Other arguments

In addition to the arguments for worker connections, you can use the following arguments for other settings:

<table>
    <tbody>
        <tr>
            <td style="width: 30%;"><strong>Argument</strong></td>
            <td style="width: 8%;"><strong>Type</strong></td>
            <td style="width: 62%;"><strong>Descrption</strong></td>
        </tr>
        <tr>
            <td style="width: 30%;"><code>-NoLogToSpatial</code></td>
            <td style="width: 8%;">flag</td>
            <td style="width: 62%;">Use this flag to stop a server-worker instance from sending warning or error log messages to SpatialOS.
                <p><strong>Note</strong>: You can add this flag to help improve performance if SpatialOS is generating a large number of warning or error log messages.</p>
            </td>
        </tr>
        <tr>
            <td style="width: 30%;"><code>-enableProtocolLogging</code></td>
            <td style="width: 8%;">bool</td>
            <td style="width: 62%;">Set to <code>true</code> to enable <a href="https://docs.improbable.io/reference/latest/shared/debugging#protocol-logging">protocol logging</a> to record any data exchanged between the worker instance and the deployment it&#39;s connected
                to, as well as other significant events.
                <p>Default:<code>false</code></p>
            </td>
        </tr>
        <tr>
            <td style="width: 30%;"><code>-protocolLoggingPrefix</code></td>
            <td style="width: 8%;">string</td>
            <td style="width: 62%;">The path that the protocol logs are generated to.
                <p><strong>Note</strong>: If you enable protocol logging, you must set this value.</p>
            </td>
        </tr>
        <tr>
            <td style="width: 30%;"><code>-OverrideSpatialNetworking</code></td>
            <td style="width: 8%;">bool</td>
            <td style="width: 62%;">Use this argument to override any other setting that you&#39;ve configured for the networking mode:
                <ul>
                    <li>When you run the command with the <code>-OverrideSpatialNetworking</code> or the <code>-OverrideSpatialNetworking=true</code> argument, you enable SpatialOS networking.</li>
                    <li>When you run the command with the <code>-OverrideSpatialNetworking=false</code> argument, you disable SpatialOS networking and use native Unreal networking instead.</li>
                </ul>
            </td>
        </tr>
    </tbody>
</table>

## Map travel URL options
In the GDK, so that a client-worker instance executes a [map travel]({{urlRoot}}/content/map-travel), you can add the following URL options to the `ClientTravel` URL:

<table style="width: 100%;">
    <tbody>
        <tr>
            <td style="width: 30%;"><strong>Option</strong></td>
            <td style="width: 8%;"><strong>Type</strong></td>
            <td style="width: 62%;"><strong>Description</strong></td>
        </tr>
        <tr>
            <td style="width: 30%;"><code>locator<code></td>
            <td style="width: 8%;">flag</td>
            <td style="width: 62%;">
                <p dir="ltr">To connect a client-worker instance to a deployment using the Locator, add this flag to the <code>ClientTravel</code> URL.</p><strong>Note</strong>: If you add this option, ensure that you also add the <code>playeridentity</code> option and the <code>login</code> option to the URL.<br></td>
        </tr>
        <tr>
            <td style="width: 30%;"><code>playeridentity</code></td>
            <td style="width: 8%;">string</td>
            <td style="width: 62%;">The <a href="https://docs.improbable.io/reference/13.7/shared/auth/integrate-authentication-platform-sdk#2-generating-a-playeridentitytoken"><code>PlayerIdentityToken</code></a> (PIT) that the client-worker instance passes to the Locator service to prove its player identity within SpatialOS.<br></td>
        </tr>
        <tr>
            <td style="width: 30%;"><code>login<code><br></td>
            <td style="width: 8%;">string</td>
            <td style="width: 62%;">The <a href="https://docs.improbable.io/reference/13.7/shared/auth/integrate-authentication-platform-sdk#3-generating-a-logintoken"><code>LoginToken</code></a> that the client-worker instance passes to the Locator service to be permitted to join the deployment.<br></td>
        </tr>
        <tr>
            <td style="width: 30%;"><code>useExternalIpForBridge</code><br></td>
            <td style="width: 8%;">bool</td>
            <td style="width: 62%;">
                <p dir="ltr">Set to <code>true</code> to connect a client-worker instance to a deployment in either of the following ways:</p>
                <ul>
                    <li dir="ltr">
                        <p dir="ltr">To connect to a deployment on a different machine from the worker instance's machine (not hosted in the cloud).</p>
                    </li>
                    <li dir="ltr">
                        <p dir="ltr">To connect to a cloud deployment using <code>spatial cloud connect external</code>.</p>
                    </li>
                </ul>
                <p dir="ltr">Default: <code>false</code></p>
            </td>
        </tr>
    </tbody>
</table>



<br/>------<br/>
_2019-06-13 Page added with editorial review_
