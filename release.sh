#!/bin/sh

if [ "$1" = "release" ]; then
  echo "Making release, uploading to PyPi and Anaconda..."
  # source dist 
  source ~/anaconda2/bin/activate root
  python setup.py sdist upload --sign
  python setup.py bdist_egg upload --sign
  
  source ~/anaconda3/bin/activate root
  python setup.py bdist_egg upload --sign
  
  # docs will be built automatically by readthedocs.org
  # python setup.py build_sphinx
  
  # Conda packages
  ~/anaconda2/bin/conda config --set anaconda_upload yes
  source ~/anaconda2/bin/activate root
  python setup.py --no-pkg-config bdist_conda
  
  ~/anaconda3/bin/conda config --set anaconda_upload yes
  source ~/anaconda3/bin/activate root
  python setup.py --no-pkg-config bdist_conda
else
  echo "Dry run, not uploading anything..."
  # source dist 
  source ~/anaconda2/bin/activate root
  python setup.py sdist
  python setup.py bdist_egg
  
  source ~/anaconda3/bin/activate root
  python setup.py sdist
  python setup.py sdist
  python setup.py bdist_egg
  
  # docs will be built automatically by readthedocs.org
  # but let's test if there is not anything going wrong
  python setup.py build_sphinx
  
  # Conda packages
  ~/anaconda2/bin/conda config --set anaconda_upload no
  source ~/anaconda2/bin/activate root
  python setup.py --no-pkg-config bdist_conda
  
  ~/anaconda3/bin/conda config --set anaconda_upload no
  source ~/anaconda3/bin/activate root
  python setup.py --no-pkg-config bdist_conda
fi

