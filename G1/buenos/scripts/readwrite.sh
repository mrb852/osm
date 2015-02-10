#!/bin/bash
cd ~/test/buenos-g1 &&
make clean &&
make &&
cd ~/test/buenos-g1/tests &&
make clean &&
make &&
cd .. &&
util/tfstool delete fyams.harddisk rw &&
util/tfstool write fyams.harddisk tests/readwrite rw &&
fyams-sim buenos initprog=[disk]rw
