#!/bin/sh

mkdir /tmp/louvain/
cp -R * /tmp/louvain/

cd /tmp/louvain/

# source dist 
source ~/anaconda3/bin/activate root
python setup.py sdist
#python setup.py sdist upload --sign
#python2.7 setup.py bdist_egg upload --sign
#python3.4 setup.py bdist_egg upload --sign

# docs will be built automatically by readthedocs.org
# python setup.py build_sphinx

# Conda packages
conda config --set anaconda_upload yes
source ~/anaconda3/bin/activate root
python setup.py --no-pkg-config bdist_conda

~/miniconda3/bin/python setup.py --no-pkg-config bdist_conda
~/miniconda3/bin/anaconda upload -i ~/miniconda3/conda-bld/linux-64/louvain-0.5.3-py35_0.tar.bz2

cd -

if [ ! -d "dist/" ]; then
  mkdir dist/
fi

cp /tmp/louvain/dist/* dist/

rm -rf /tmp/louvain/
