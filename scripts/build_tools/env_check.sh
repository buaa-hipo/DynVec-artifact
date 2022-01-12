#!/bin/bash
set -e
echo "Checking oneAPI utilities ..."
if ! command -v icc #&> /dev/null
then
  echo "oneAPI not enabled or installed!"
  exit -1
fi
