#!/bin/bash
if ! [ -d "data" ]; then
  if ! [ -f "data.tar.gz" ]; then
    echo "Downloading Data for DynVec Evaluation..."
  else
    echo "Use the cached data.tar.gz"
  fi
  echo "Extracting data..."
  tar xvzf data.tar.gz
else
  echo "Data already prepared. Skip downloading and extracting data."
fi
