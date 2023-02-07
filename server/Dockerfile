# Update and install cross-platform build dependencies
FROM gcc:latest

# Maintainer info
MAINTAINER Noam DE MASURE <demasure.noam@hotmail.fr>

COPY . /DockerWorld

WORKDIR /DockerWorld/

RUN apt update && apt upgrade -y
RUN apt install -y libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
RUN  ./server/script.sh
