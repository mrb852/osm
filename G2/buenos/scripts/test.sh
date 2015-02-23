#!/bin/bash
cd ~/test/shared/G2/buenos/buenos-g2 &&
make clean &&
make &&
cd ~/test/shared/G2/buenos/buenos-g2/tests &&
make clean &&
make &&
cd .. &&
util/tfstool delete fyams.harddisk process_test
util/tfstool delete fyams.harddisk nojob
util/tfstool write fyams.harddisk tests/process_test process_test &&
util/tfstool write fyams.harddisk tests/nojob nojob &&
fyams-sim buenos initprog=[disk]process_test
