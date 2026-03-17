# Copyright(C) 2026, Proyectos y Sistemas de Mantenimiento SL(eProsima)
#
#  This program is commercial software licensed under the terms of the
#  eProsima Software License Agreement Rev 03 (the "License")
#
#  You may obtain a copy of the License at
#  https://www.eprosima.com/licenses/LICENSE-REV03

ARG ubuntu_version=24.04
FROM ubuntu:$ubuntu_version AS ubuntu-net-ip-tools

# Needed for a dependency that forces to set timezone
ENV TZ=Europe/Madrid
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Avoids using interactions during building
ENV DEBIAN_FRONTEND=noninteractive

# Install apt dependencies
RUN apt-get update && \
    apt-get install --yes net-tools iproute2 iptables libtinyxml2-dev && \
    rm -rf /var/lib/apt/lists/*
