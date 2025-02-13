#########################################################################################
# Vulcanexus Dockerfile
#########################################################################################

FROM eprosima/vulcanexus:jazzy-base

WORKDIR /vulcanexus_ws

SHELL ["/bin/bash", "-c"]

RUN apt update && \
    apt install lsof iproute2 ros-jazzy-demo-nodes-cpp ros-jazzy-turtlesim vim gdb -y
# Install Nav2 stack
RUN source /opt/ros/jazzy/setup.bash && \
    apt install -y ros-jazzy-navigation2 ros-jazzy-nav2-bringup ros-jazzy-nav2-minimal-tb*

# Clone repositories
RUN wget https://raw.githubusercontent.com/eProsima/Fast-DDS/refs/heads/master/fastdds.repos && \
    mkdir src && \
    vcs import src < fastdds.repos

# Select fastdds branch
RUN cd src/fastdds && \
    git checkout master

RUN git clone https://github.com/eProsima/rmw_fastrtps src/rmw -b vulcanexus-jazzy

# Build
RUN colcon build --packages-up-to fastdds --cmake-args -DCMAKE_BUILD_TYPE=Debug -DTHIRDPARTY_Asio=FORCE -DCOMPILE_TOOLS=ON -DINSTALL_TOOLS=ON -DCOMPILE_EXAMPLES=ON -DINSTALL_EXAMPLES=ON -DCMAKE_CXX_FLAGS="-Wno-ignored-attributes"

RUN source /opt/vulcanexus/jazzy/setup.bash && \
    source /vulcanexus_ws/install/setup.bash && \
    colcon build --packages-up-to rmw_fastrtps_cpp --cmake-args -DBUILD_TESTING=OFF


# Set Fast DDS as middleware
ENV RMW_IMPLEMENTATION=rmw_fastrtps_cpp
ENV TURTLEBOT3_MODEL=waffle
ENV GAZEBO_MODEL_PATH=$GAZEBO_MODEL_PATH:/opt/ros/jazzy/share/turtlebot3_gazebo/models

# Setup entrypoint
COPY ./vulcanexus_entrypoint.sh /
ENTRYPOINT ["/bin/bash", "/vulcanexus_entrypoint.sh"]
CMD ["bash"]