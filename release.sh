#!/bin/sh

if [ "$1" = "release" ]; then
  echo "Making release, uploading to PyPi and Anaconda..."
  
  # Python 2
  source ~/anaconda2/bin/activate root
  ~/anaconda2/bin/conda config --set anaconda_upload yes  
  python setup.py sdist upload --sign         # Source
  python setup.py bdist_egg upload --sign     # Binary
  python setup.py --no-pkg-config bdist_conda # Conda binary

  # Python 3 (source only needed once)
  source ~/anaconda3/bin/activate root
  ~/anaconda3/bin/conda config --set anaconda_upload yes  
  python setup.py bdist_egg upload --sign     # Binary
  python setup.py --no-pkg-config bdist_conda # Conda binary

  # docs will be built automatically by readthedocs.org
  # python setup.py build_sphinx
  
else
  echo "Dry run, not uploading anything..."
  
  # Python 2
  source ~/anaconda2/bin/activate root
  ~/anaconda2/bin/conda config --set anaconda_upload no
  python setup.py sdist                       # Source
  python setup.py bdist_egg                   # Binary
  python setup.py --no-pkg-config bdist_conda # Conda binary

  # Python 3 (source only needed once)
  source ~/anaconda3/bin/activate root
  ~/anaconda3/bin/conda config --set anaconda_upload no
  python setup.py bdist_egg                   # Binary
  python setup.py --no-pkg-config bdist_conda # Conda binary

  # Install & test
  python setup.py install  
  python setup.py test

  # docs will be built automatically by readthedocs.org
  python setup.py build_sphinx
  sphinx-build -b doctest -d doc/build/doctrees/ doc/source/ doc/build/

fi

