# Copyright (C) 2026, Proyectos y Sistemas de Mantenimiento SL (eProsima)
#
# This program is commercial software licensed under the terms of the
# eProsima Software License Agreement Rev 03 (the "License")
#
# You may obtain a copy of the License at
# https://www.eprosima.com/licenses/LICENSE-REV03

#!/usr/bin/env bash
set -euo pipefail

DOCKER="@DOCKER_EXECUTABLE@"
DEFAULT_PROJECT_NAME="dynif_@COMPOSE_PROJECT@"

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <compose_file> [project_name] [profile_name]" >&2
  exit 2
fi

COMPOSE_FILE="$1"
PROJECT_NAME="${2:-$DEFAULT_PROJECT_NAME}"
PROFILE_NAME="${3:-}"
# If set, target host daemon
DOCKER_HOST_RESOLVED="@DOCKER_HOST_RESOLVED@"

COMPOSE_PROFILES=()
if [[ -n "${PROFILE_NAME}" ]]; then
  COMPOSE_PROFILES+=(--profile "${PROFILE_NAME}")
fi

# Rely on DOCKER_HOST environment variable rather than use -H flag
if [[ -n "${DOCKER_HOST_RESOLVED}" ]]; then
  export DOCKER_HOST="${DOCKER_HOST_RESOLVED}"
fi

# Always cleanup
cleanup() {
  set +e
  "${DOCKER}" compose --project-name "${PROJECT_NAME}" -f "${COMPOSE_FILE}" "${COMPOSE_PROFILES[@]}" down -v --remove-orphans
}
trap cleanup EXIT

"${DOCKER}" compose \
  --project-name "${PROJECT_NAME}" \
  "${COMPOSE_PROFILES[@]}" \
  -f "${COMPOSE_FILE}" up \
  --abort-on-container-exit \
  --exit-code-from receiver
