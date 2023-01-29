# Base image for msbuild (uses Mono)


# Maintainer info
MAINTAINER Noam DE MASURE <demasure.noam@hotmail.fr>

# Update and install cross-platform build dependencies
RUN apt-get install -y \
    gcc \
    libsdl2-dev \
    libsdl2-image-dev \
    libsdl2-ttf-dev \
    mingw-w64 \
    cmake \
