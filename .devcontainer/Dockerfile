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
 && python3 -m pip install cookiecutter sphinx breathe sphinx_rtd_theme

ENV DEVUSER=${DEVUSER}
RUN adduser --uid 1000 --disabled-password --gecos "" ${DEVUSER} \
    && echo "${DEVUSER} ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/${DEVUSER}
USER ${DEVUSER}
