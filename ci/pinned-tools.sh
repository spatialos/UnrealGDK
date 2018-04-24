#!/usr/bin/env bash

# Function declarations

error() {
   local SOURCE_FILE=$1
   local LINE_NO=$2
   echo "ERROR: ${SOURCE_FILE}: ${LINE_NO}"
}
trap 'error "${BASH_SOURCE}" "${LINENO}"' ERR

if [ -z "$IMPROBABLE_TOOLS" ]; then
    echo "The internal tools share is not set up correctly on this machine. Please follow the setup instructions here before running build.sh: https://brevi.link/internal-tools-share"
    exit 1
fi

function isLinux() {
  [[ "$(uname -s)" == "Linux" ]];
}

function isMacOS() {
  [[ "$(uname -s)" == "Darwin" ]];
}

function isWindows() {
  ! ( isLinux || isMacOS );
}

function getPlatformName() {
  if isLinux; then
    echo "linux"
  elif isMacOS; then
    echo "mac"
  elif isWindows; then
    echo "windows"
  else
    echo "ERROR: Unknown platform." >&2
    exit 1
  fi
}

TOOLS_OS="$(getPlatformName)"

function isTeamCity() {
  # -n == string comparison "not null"
  [ -n "${TEAMCITY_CAPTURE_ENV+x}" ]
}

function isNotTeamCity() {
  # -z == string comparison "null, that is, 0-length"
  [ -z "${TEAMCITY_CAPTURE_ENV+x}" ]
}

function unpackTo() {
  local SOURCE=$1
  local TARGET=$2

  mkdir -p "${TARGET}"
  unzip -o -q "${SOURCE}" -d "${TARGET}"
}

function unpackToWithClean() {
  local SOURCE=$1
  local TARGET=$2

  rm -rf "${TARGET}"
  unpackTo "${SOURCE}" "${TARGET}"
}

function runSpatial() {
  local default_flags=(
    "--log_level=debug"
  )

  forceSpatialCliStructureV2
  spatial "$@" "${default_flags[@]}"
}

# Variable declarations

PROGRAMFILES_X86=$(cmd.exe /c "echo %ProgramFiles(x86)%")
MSBUILD="${PROGRAMFILES_X86}\MSBuild\14.0\Bin\MSBuild.exe"

IMP_NUGET_VERSION="20180320.121538.4d07aa9573"
IMP_NUGET="${IMPROBABLE_TOOLS}/imp-nuget/${IMP_NUGET_VERSION}/${TOOLS_OS}/imp-nuget"

IMP_WORKER_PACKAGE_VERSION="20180320.120511.bb22a194cb"
IMP_WORKER_PACKAGE="${IMPROBABLE_TOOLS}/imp-worker-package/${IMP_WORKER_PACKAGE_VERSION}/${TOOLS_OS}/imp-worker-package"

PACKAGE_CLIENT_VERSION="20171115.142004.8707ef0"
PACKAGE_CLIENT="${IMPROBABLE_TOOLS}/package_client/${PACKAGE_CLIENT_VERSION}/${TOOLS_OS}/package_client"

IMP_LINT_VERSION="20171129.134829.183d8f6"
IMP_LINT_BIN="${IMPROBABLE_TOOLS}/imp_lint/${IMP_LINT_VERSION}/${TOOLS_OS}/imp_lint"

REGISSEUR_VERSION="20180404.151549.371ed042ca"
REGISSEUR_BIN="${IMPROBABLE_TOOLS}/regisseur/${REGISSEUR_VERSION}/${TOOLS_OS}/regisseur"

# # The deprecated version of Unreal. We still want to make sure it compiles, but it's not tested any further than that.
# PREVIOUS_UNREAL_VERSION="4.16.3"
# PREVIOUS_UNREAL_HOME="C:/Unreal/UnrealEngine-${PREVIOUS_UNREAL_VERSION}"

SCHEMA_COMPILER_PACKAGE="schema_compiler-x86_64-win32"

# if isWindows ; then
#     SCHEMA_COMPILER_PACKAGE="schema_compiler-x86_64-win32"
# elif isMacOS ; then
#     SCHEMA_COMPILER_PACKAGE="schema_compiler-x86_64-macoswin32"
# else
#     echo "This platform is not supported".
#     exit 1
# fi

# # The current version of Unreal.
# UNREAL_VERSION="4.17.1"
# UNREAL_HOME="C:/Unreal/UnrealEngine-${UNREAL_VERSION}"

# # LINUX_MULTIARCH_ROOT is used by Unreal when cross compiling Linux workers
# # as Unreal only builds on windows otherwise. The Linux cross compiling tools
# # should be automatically installed by puppet as part of the Unreal Engine
# # installation.
# LINUX_MULTIARCH_ROOT="${UNREAL_HOME}/ClangToolchain"

GOPATH="$(pwd)/go"

export GOPATH
# export UNREAL_HOME
# export LINUX_MULTIARCH_ROOT
