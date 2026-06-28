FROM mathemaphysics/cmake-base:3.22

ARG DEVUSER devuser

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get -y update \
 && apt-get -y install --no-install-recommends build-essential \
 					       g++ \
					       gdb \
						   doxygen \
					       git \
					       openssh-client \
						   openssl \
						   libssl-dev \
					       wget \
					       vim \
					       sudo \
					       libboost-all-dev \
					       libopencv-dev \
					       python3 \
					       python3-dev \
					       python3-pip \
					       python3-numpy \
 && apt-get -y remove cmake \
 && python3 -m pip install cookiecutter sphinx breathe sphinx_rtd_theme

# Build and install the latest CMake release from source so the devcontainer
# has a recent CMake (with workflow/preset support) available for building.
ARG CMAKE_VERSION=3.29.6
RUN cd /tmp \
 && wget -q https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}.tar.gz \
 && tar -xzf cmake-${CMAKE_VERSION}.tar.gz \
 && cd cmake-${CMAKE_VERSION} \
 && ./bootstrap --parallel=$(nproc) -- -DCMAKE_USE_OPENSSL=ON \
 && make -j$(nproc) \
 && make install \
 && cd /tmp \
 && rm -rf cmake-${CMAKE_VERSION} cmake-${CMAKE_VERSION}.tar.gz

ENV DEVUSER=devuser
RUN adduser --uid 1000 --disabled-password --gecos "" ${DEVUSER} \
    && echo "${DEVUSER} ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/${DEVUSER}
USER ${DEVUSER}
