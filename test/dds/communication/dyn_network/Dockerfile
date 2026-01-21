# Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

# Tag, branch, or commit in github.com/eProsima/DDS-Suite
ARG ubuntu_version=22.04
FROM ubuntu:$ubuntu_version AS ubuntu-net-tools

# Needed for a dependency that forces to set timezone
ENV TZ=Europe/Madrid
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Avoids using interactions during building
ENV DEBIAN_FRONTEND=noninteractive

# Install apt dependencies
RUN apt-get update && apt-get install --yes net-tools && rm -rf /var/lib/apt/lists/*

FROM ubuntu:$ubuntu_version AS ubuntu-cli

# Needed for a dependency that forces to set timezone
ENV TZ=Europe/Madrid
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Avoids using interactions during building
ENV DEBIAN_FRONTEND=noninteractive

# Install apt dependencies
RUN apt-get update && apt-get install --yes python3 python3-pip && rm -rf /var/lib/apt/lists/*

<<<<<<< HEAD:test/dds/communication/dyn_network/Dockerfile
RUN pip3 install psutil
=======
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

ENTRYPOINT ["/bin/bash", "-c", "cd && colcon build --event-handlers=console_direct+ --cmake-args -DSECURITY=ON -DFASTDDS_STATISTICS=ON"]
>>>>>>> 8b28dabf3 (Fix build in GCC 15 (#6263)):.github/workflows/docker/ubuntu/alternative_builds/fastdds_gcc_latest.Dockerfile
