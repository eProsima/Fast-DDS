# Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

FROM gcc:latest

# Needed for a dependency that forces to set timezone
ENV TZ=Europe/Madrid
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Avoids using interactions during building
ENV DEBIAN_FRONTEND=noninteractive

# Use a bash shell so it is possigle to run things like `source` (required for colcon builds)
SHELL ["/bin/bash", "-c"]

# Install apt dependencies
RUN apt-get update && apt-get install --yes --no-install-recommends \
    # Common build dependencies
    build-essential \
    cmake \
    git \
    python3-pip \
    wget

# Install required python modules
RUN rm /usr/lib/python*/EXTERNALLY-MANAGED && \
    pip3 install \
    colcon-common-extensions \
    vcstool

# Install apt dependencies
RUN apt-get update && apt-get install --yes --no-install-recommends \
    # Fast DDS dependencies
    libssl-dev \
    libasio-dev \
    libtinyxml2-dev \
    openssl

ENTRYPOINT ["/bin/bash", "-c", "cd && colcon build --event-handlers=console_direct+"]
