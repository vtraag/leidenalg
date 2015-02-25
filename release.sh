#!/bin/sh

mkdir /tmp/louvain/
cp -R * /tmp/louvain/

cd /tmp/louvain/

# source dist 
python setup.py sdist
#python setup.py sdist upload --sign
#python2.7 setup.py bdist_egg upload --sign
#python3.4 setup.py bdist_egg upload --sign

cd -

if [ ! -d "dist/" ]; then
  mkdir dist/
fi

cp /tmp/louvain/dist/* dist/

rm -rf /tmp/louvain/
