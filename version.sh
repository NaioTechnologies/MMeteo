#!/bin/bash

cp $1/include/version.h.template $1/include/version.h

git -C $1 rev-parse HEAD | (read f;sed -i 's/X1X/'$f'/g' $1/include/version.h)

( hostname ; hostid ) | (read f;read g;sed -i 's/X2X/'$f' '$g'/g' $1/include/version.h)

date +"%Y-%m-%d-%H-%M-%S" | (read f;sed -i 's/X3X/'$f'/g' $1/include/version.h)
