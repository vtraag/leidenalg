#!/bin/sh

mkdir /tmp/louvain/
cp -R * /tmp/louvain/

cd /tmp/louvain/

# source dist 
python setup.py sdist
#python setup.py sdist upload --sign
#python2.7 setup.py bdist_egg upload --sign
#python3.4 setup.py bdist_egg upload --sign

# Conda packages
~/miniconda2/bin/python setup.py --no-pkg-config bdist_conda
~/miniconda2/bin/anaconda upload -i ~/miniconda2/conda-bld/linux-64/louvain-0.5.3-py27_0.tar.bz2

~/miniconda3/bin/python setup.py --no-pkg-config bdist_conda
~/miniconda3/bin/anaconda upload -i ~/miniconda3/conda-bld/linux-64/louvain-0.5.3-py35_0.tar.bz2

cd -

if [ ! -d "dist/" ]; then
  mkdir dist/
fi

cp /tmp/louvain/dist/* dist/

rm -rf /tmp/louvain/
