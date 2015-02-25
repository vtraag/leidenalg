#!/bin/sh

mkdir /tmp/louvain/
cp -R * /tmp/louvain/

cd /tmp/louvain/

# source dist 
python setup.py sdist upload --sign
python setup.py bdist_egg upload --sign

rm -rf /tmp/louvain/
