#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT_DIR"

SOCKET_PREFIX="mirrordesk"
BASE_PORT="27183"
WIDTH="1280"
HEIGHT="720"
BITRATE="6000000"
MAXFPS="60"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --serial) shift 2 ;;
    --socket) SOCKET_PREFIX="$2"; shift 2 ;;
    --port|--base-port) BASE_PORT="$2"; shift 2 ;;
    --w) WIDTH="$2"; shift 2 ;;
    --h) HEIGHT="$2"; shift 2 ;;
    --bitrate) BITRATE="$2"; shift 2 ;;
    --maxfps) MAXFPS="$2"; shift 2 ;;
    *) echo "$1"; exit 1 ;;
  esac
done

ADB_BIN="${ADB:-adb}"
command -v "$ADB_BIN" >/dev/null 2>&1 || { echo "adb" >&2; exit 1; }

if [[ -z "${JAVA_HOME:-}" || -z "${JDK_HOME:-}" ]]; then
  if [[ -d "$HOME/.sdkman/candidates/java/current" ]]; then
    export JAVA_HOME="$(readlink -f "$HOME/.sdkman/candidates/java/current")"
  fi
fi
if [[ -z "${JDK_HOME:-}" && -n "${JAVA_HOME:-}" ]]; then export JDK_HOME="$JAVA_HOME"; fi
if [[ -z "${JAVA_HOME:-}" || -z "${JDK_HOME:-}" ]]; then echo "JDK" >&2; exit 1; fi

GRADLE_CMD=("$ROOT_DIR/gradlew")
JAVA_HOME="$JAVA_HOME" JDK_HOME="$JDK_HOME" "${GRADLE_CMD[@]}" :androidServer:assembleRelease
JAVA_HOME="$JAVA_HOME" JDK_HOME="$JDK_HOME" "${GRADLE_CMD[@]}" :androidServer:exportServerJar

JAR_PATH="$ROOT_DIR/androidServer/build/dist/mirrordesk-android-server.jar"
[[ -f "$JAR_PATH" ]] || { echo "$JAR_PATH" >&2; exit 1; }

mapfile -t DEVICES < <("$ADB_BIN" devices | awk '/\tdevice$/{print $1}')
[[ ${#DEVICES[@]} -gt 0 ]] || { echo "no devices" >&2; exit 1; }

idx=0
for serial in "${DEVICES[@]}"; do
  port=$((BASE_PORT + idx))
  socket_name="${SOCKET_PREFIX}_${idx}"
  "$ADB_BIN" -s "$serial" push "$JAR_PATH" /data/local/tmp/mirrordesk-android-server.jar >/dev/null
  "$ADB_BIN" -s "$serial" forward --remove "tcp:${port}" >/dev/null 2>&1 || true
  "$ADB_BIN" -s "$serial" forward "tcp:${port}" "localabstract:${socket_name}"
  start_cmd="CLASSPATH=/data/local/tmp/mirrordesk-android-server.jar app_process / com.mirrordesk.androidserver.MiniServer --socket ${socket_name} --w ${WIDTH} --h ${HEIGHT} --bitrate ${BITRATE} --maxfps ${MAXFPS}"
  "$ADB_BIN" -s "$serial" shell nohup sh -c "$start_cmd </dev/null >/dev/null 2>&1 &" || true
  idx=$((idx+1))
done

JAVA_HOME="$JAVA_HOME" JDK_HOME="$JDK_HOME" "${GRADLE_CMD[@]}" :composeApp:run


