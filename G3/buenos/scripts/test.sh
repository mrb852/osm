#!/bin/bash
cd ~/test/shared/G3/buenos/buenos-g3 &&
make &&
cd ~/test/shared/G3/buenos/buenos-g3/tests &&
make &&
cd .. &&
util/tfstool delete fyams.harddisk halt
util/tfstool write fyams.harddisk tests/halt halt &&
fyams-sim buenos initprog=[disk]halt
