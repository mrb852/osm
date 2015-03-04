#!/bin/bash
cd ~/test/shared/G4/buenos/buenos-g4 &&
make &&
cd ~/test/shared/G4/buenos/buenos-g4/tests &&
make &&
cd .. &&
<<<<<<< HEAD
util/tfstool delete fyams.harddisk halt
util/tfstool write fyams.harddisk tests/halt halt &&
fyams-sim buenos initprog=[disk]halt
=======
util/tfstool delete fyams.harddisk spawner
util/tfstool delete fyams.harddisk kat
util/tfstool delete fyams.harddisk mus
util/tfstool write fyams.harddisk tests/spawner spawner &&
util/tfstool write fyams.harddisk tests/kat kat &&
util/tfstool write fyams.harddisk tests/mus mus &&
fyams-sim buenos initprog=[disk]spawner
>>>>>>> d11bf8fc946296d1d246c173aa516ea61737f06b
