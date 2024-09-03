#!/bin/bash

# this script is intended to be run from the root of the project

genblog_file="scripts/genblog.cc"
compile_flags_file="scripts/compile_flags.txt"
compile_flags=$(cat "$compile_flags_file")

c++ $compile_flags "$genblog_file" -o scripts/build/genblog

# generate the post index
scripts/build/genblog