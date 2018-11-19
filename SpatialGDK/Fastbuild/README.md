# Introduction
[FASTBuild](http://www.fastbuild.org/docs/home.html) is a distributed build and caching system.

Improbable have integrated with the Unreal Build Tool, using [Unreal_FASTBuild](https://github.com/liamkf/Unreal_FASTBuild)
as the foundation.

# Installation

FASTBuild can be installed in two different ways:
* As a service, which is good for build agents.
* As a GUI, which is good for your local machine.

> All of these must be run from an **administrator** `powershell` or `cmd` prompt.

## As a service
FASTBuild can be installed as a service, for build agents and other non-interactive scenarios.

**To install**

  `powershell -NoProfile -ExecutionPolicy Bypass -File Install.ps1 -service`

**To uninstall**

  `powershell -NoProfile -ExecutionPolicy Bypass -File Uninstall.ps1 -service`

# As a GUI
If you're installing on your workstation, it's recommended to install it in interactive mode.

**To install**

  `powershell -NoProfile -ExecutionPolicy Bypass -File Install.ps1`

**To uninstall**

  `powershell -NoProfile -ExecutionPolicy Bypass -File Uninstall.ps1`

# Useful tools

* A Visual Studio plugin for monitoring the status of builds: [FASTBuildMonitor](https://github.com/yass007/FASTBuildMonitor)
* An alternative, standalone build monitor: [FASTBuild-Dashboard](https://github.com/hillin/FASTBuild-Dashboard)

