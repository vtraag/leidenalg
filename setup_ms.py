from setuptools import setup, Extension
import glob
import os

# Utility function to read the README file.
# Used for the long_description.  It's nice, because now 1) we have a top level
# README file and 2) it's easier to type in the README file than to put a raw
# string in below ...
def read(fname):
    return open(os.path.join(os.path.dirname(__file__), fname)).read()


import sys
# Check if working with Python 3
PY3 = (sys.version > '3');
# Check if 64 bit or not
x64 = (sys.maxsize > 2**32);

print("Building for " + ( "Python 3" if PY3 else "Python 2" ) + " with a " + ( "64" if x64 else "32" ) + " bits architecture.");

if PY3:
  if x64:
    inc_dir = 'igraph-0.7.0-msvc-py3/include';
    lib_dir = 'igraph-0.7.0-msvc-py3/Release/x64';
  else:
    inc_dir = 'igraph-0.7.0-msvc-py3/include';
    lib_dir = 'igraph-0.7.0-msvc-py3/Release/win32';
else:
  if x64:
    inc_dir = 'igraph-0.7.0-msvc-py2.7/include';
    lib_dir = 'igraph-0.7.0-msvc-py2.7/Release/x64';
  else:
    inc_dir = 'igraph-0.7.0-msvc-py2.7/include';
    lib_dir = 'igraph-0.7.0-msvc-py2.7/Release/win32';

print("Using include dir: " + inc_dir);
print("Using library dir: " + lib_dir);

louvain_ext = Extension('louvain._c_louvain',
                    sources = glob.glob(os.path.join('src', '*.cpp')),
                    libraries = ['igraph'],
                    include_dirs=['include', inc_dir],
                    library_dirs=['wlib', lib_dir]);

options =  dict(
  name = 'louvain',
  version = '0.5.2',
  description = 'Louvain is a general algorithm for methods of community detection in large networks.',
  long_description=read('README.md'),
  license = 'GPLv3+',
  url = 'https://github.com/vtraag/louvain-igraph',

  author = 'V.A. Traag',
  author_email = 'vincent@traag.net',

  provides = ['louvain'],
  package_dir = {'louvain': 'src'},
  packages = ['louvain'],
  ext_modules = [louvain_ext],
  classifiers=[
      'Development Status :: 4 - Beta',
      'Environment :: Console',
      'Intended Audience :: End Users/Desktop',
      'Intended Audience :: Developers',
      'Intended Audience :: Science/Research',
      'License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)',
      'Operating System :: MacOS :: MacOS X',
      'Operating System :: Microsoft :: Windows',
      'Operating System :: POSIX',
      'Programming Language :: Python',
      'Programming Language :: C++',
      'Topic :: Scientific/Engineering :: Mathematics',
      'Topic :: Scientific/Engineering :: Information Analysis',
      'Topic :: Sociology'
    ]);

setup(**options);
