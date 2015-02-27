#!/bin/bash
cd ~/test/shared/G3/buenos/buenos-g3 &&
make &&
cd ~/test/shared/G3/buenos/buenos-g3/tests &&
make &&
cd .. &&
util/tfstool delete fyams.harddisk spawner
util/tfstool delete fyams.harddisk kat
util/tfstool delete fyams.harddisk mus
util/tfstool write fyams.harddisk tests/spawner spawner &&
util/tfstool write fyams.harddisk tests/kat kat &&
util/tfstool write fyams.harddisk tests/mus mus &&
fyams-sim buenos initprog=[disk]spawner
