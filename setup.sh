#! bin/bash

cd ..
echo "--- Intall Dependencies for Linux Ubuntu ---"
	CC=`lsb_release -rs | cut -c 1-2`
	VER=`expr $CC + 0`
	if [[ $VER -gt 15 ]]; then
		apt-get install -y software-properties-common
		apt-get update
		apt-get install -y cmake git build-essential libssl-dev libgmp-dev python 
		apt-get install -y libboost-dev
		apt-get install -y libboost-{chrono,log,program-options,date-time,thread,system,filesystem,regex,test}-dev
	else
		apt-get install -y software-properties-common
		add-apt-repository -y ppa:george-edison55/cmake-3.x
		add-apt-repository -y ppa:kojoley/boost
		apt-get update
		apt-get install -y cmake git build-essential libssl-dev libgmp-dev python 
		apt-get install -y libboost-all-dev
#		sudo apt-get install -y libboost1.58-dev
#		sudo apt-get install -y libboost-{chrono,log,program-options,date-time,thread,system,filesystem,regex,test}1.58-dev
	fi

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
