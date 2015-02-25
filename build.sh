#!/bin/sh

mkdir /tmp/louvain/
cp -R * /tmp/louvain/

cd /tmp/louvain/

# source dist for python 2.7
python2.7 setup.py sdist

# source dist for python 3.4
python3.4 setup.py sdist

# Switch back to previous directory
cd -

# Ensure dir exists
mkdir dist/

# Copy distibution files
cp /tmp/louvain/dist/* dist/

rm -rf /tmp/louvain/
