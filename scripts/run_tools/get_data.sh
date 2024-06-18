#!/bin/bash
if ! [ -d "data" ]; then
  if ! [ -f "data.tar.gz" ]; then
    echo "Downloading Data for DynVec Evaluation..."
    wget -O `pwd`/data.tar.gz "https://j1btaw.bn.files.1drv.com/y4mBRQ5ZXex9V735O6t7U1FVqdY1ZS2LHNDF0fSqQVHiCgNWeFdiaZ12Cvam8sp6CyqNq97x25nKxYzXSuiqIeG2_GZAtf-B_8tN79KVXSlGXZFVPlje8soLtCbkDdRMcrHkfeNT-sbLv1LJWiEBplg5k8B1vCGpx3oUpR5mclidG6no94JOh4BPashmQPp0HuTQv7ib5fdDLWNvk1aaM8y8Q"
  else
    echo "Use the cached data.tar.gz"
  fi
  echo "Extracting data..."
  tar xvzf data.tar.gz
else
  echo "Data already prepared. Skip downloading and extracting data."
fi
