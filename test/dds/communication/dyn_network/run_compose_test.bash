# Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#!/usr/bin/env bash
set -euo pipefail

DOCKER="@DOCKER_EXECUTABLE@"
DEFAULT_PROJECT_NAME="dynif_@COMPOSE_PROJECT@"

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <compose_file> [project_name]" >&2
  exit 2
fi

COMPOSE_FILE="$1"
PROJECT_NAME="${2:-$DEFAULT_PROJECT_NAME}"
# If set, target host daemon
DOCKER_HOST_RESOLVED="@DOCKER_HOST_RESOLVED@"

# Rely on DOCKER_HOST environment variable rather than use -H flag
if [[ -n "${DOCKER_HOST_RESOLVED}" ]]; then
  export DOCKER_HOST="${DOCKER_HOST_RESOLVED}"
fi

# Always cleanup
cleanup() {
  set +e
  "${DOCKER}" compose --project-name "${PROJECT_NAME}" -f "${COMPOSE_FILE}" down -v --remove-orphans
}
trap cleanup EXIT

# Force to build without cache to ensure all changes are included
"${DOCKER}" compose \
  --project-name "${PROJECT_NAME}_build" \
  -f "${COMPOSE_FILE}" build --no-cache \
  >/dev/null

"${DOCKER}" compose \
  --project-name "${PROJECT_NAME}" \
  -f "${COMPOSE_FILE}" up \
  --abort-on-container-exit \
  --exit-code-from server-pub-sub
