#!/bin/bash
DIR=ORLIB/ORLIB-uncap
for FOLDER in `ls $DIR`
do
  for FILE in `cat $DIR/$FOLDER/files.lst`
  do
    ./bazel-bin/math_opt_benchmark/facility/ufl_main -filename=$DIR/$FOLDER/$FILE | diff $DIR/$FOLDER/$FILE.opt -
  done
done