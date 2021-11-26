#! /bin/bash

cd ..
echo "--- Intall Dependencies for Linux Ubuntu ---"

apt-get install -y software-properties-common
apt-get update
apt-get install -y cmake git build-essential libssl-dev libgmp-dev python wget
apt-get install -y libboost-dev
apt-get install -y libboost-{chrono,log,program-options,date-time,thread,system,filesystem,regex,test}-dev

echo "--- Fetch EMP-lib ---"
wget https://raw.githubusercontent.com/emp-toolkit/emp-readme/master/scripts/install.py
python install.py --deps --tool --ot --sh2pc

echo "--- Fetch TurboPFor ---"
cd CRGC-dep/Reusable-Garbled-Circuit-Generator-CPP
git submodule update --init --recursive

echo "--- Build CRGC ---"
cmake . -B ./build
cd build
make -j