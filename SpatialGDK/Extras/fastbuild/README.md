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

* To install: `powershell -NoProfile -ExecutionPolicy Bypass -File "\\filesharing2\files\fastbuild\install.ps1" -service`
* To uninstall: `powershell -NoProfile -ExecutionPolicy Bypass -File "\\filesharing2\files\fastbuild\uninstall.ps1" -service`

# As a GUI
* To install: `powershell -NoProfile -ExecutionPolicy Bypass -File "\\filesharing2\files\fastbuild\install.ps1"`
* To uninstall: `powershell -NoProfile -ExecutionPolicy Bypass -File "\\filesharing2\files\fastbuild\uninstall.ps1"`

# Useful tools

* A Visual Studio plugin for monitoring the status of builds: [FASTBuildMonitor](https://github.com/yass007/FASTBuildMonitor)
* An alternative, standalone build monitor: [FASTBuild-Dashboard](https://github.com/hillin/FASTBuild-Dashboard)

