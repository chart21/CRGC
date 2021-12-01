FROM ubuntu:20.04
WORKDIR /home
RUN apt update
RUN apt install -y sudo
RUN apt install -y git
RUN DEBIAN_FRONTEND="noninteractive" apt-get -y install tzdata
RUN apt install -y cmake
RUN mkdir CRGC-dep
WORKDIR /home/CRGC-dep/
RUN git clone https://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@github.com/chart21/Reusable-Garbled-Circuit-Generator-CPP.git
WORKDIR /home/CRGC-dep/Reusable-Garbled-Circuit-Generator-CPP/
RUN git checkout -b transfer origin/transfer
RUN ./setup.sh