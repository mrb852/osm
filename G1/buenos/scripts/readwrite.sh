#!/bin/bash
cd ~/test/shared/G1/buenos/buenos-g1 &&
make clean &&
make &&
cd ~/test/shared/G1/buenos/buenos-g1/tests &&
make clean &&
make &&
cd .. &&
util/tfstool delete fyams.harddisk rw &&
util/tfstool write fyams.harddisk tests/readwrite rw &&
fyams-sim buenos initprog=[disk]rw
