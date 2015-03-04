#!/bin/bash
cd ~/test/shared/G4/buenos/buenos-g4 &&
make &&
cd ~/test/shared/G4/buenos/buenos-g4/tests &&
make &&
cd .. &&
util/tfstool delete fyams.harddisk halt
util/tfstool write fyams.harddisk tests/halt halt &&
fyams-sim buenos initprog=[disk]halt
