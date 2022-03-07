#!/bin/bash

mkdir -p data

# Facility Location
curl --output ORLIB.tar.gz https://resources.mpi-inf.mpg.de/departments/d1/projects/benchmarks/UflLib/data/bench/ORLIB.tgz
tar -xf ORLIB.tar.gz --directory data/ORLIB
rm -f ORLIB.tar.gz

# Cutting Stock - visit https://sites.google.com/site/shunjiumetani/file/fiber.zip
