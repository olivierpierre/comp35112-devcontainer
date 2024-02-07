FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y build-essential valgrind gdb \
    manpages-dev openjdk-21-jdk openjdk-21-jre mpich

RUN rm /etc/apt/apt.conf.d/docker-clean

COPY comp35112-devcontainer /root/

WORKDIR /root