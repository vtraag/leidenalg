from setuptools import setup, Extension
import glob
import os

# Utility function to read the README file.
# Used for the long_description.  It's nice, because now 1) we have a top level
# README file and 2) it's easier to type in the README file than to put a raw
# string in below ...
def read(fname):
    return open(os.path.join(os.path.dirname(__file__), fname)).read()

louvain_ext = Extension('louvain._c_louvain',
                    sources = glob.glob(os.path.join('src', '*.cpp')),
                    libraries = ['igraph'],
                    include_dirs=['include', 'igraph-0.7.0-msvc/include'],
                    library_dirs=['wlib', 'igraph-0.7.0-msvc/Release/x64']);

options =  dict(
  name = 'louvain',
  version = '0.5',
  description = 'Louvain is a general algorithm for methods of community detection in large networks.',
  long_description=read('README.md'),
  license = 'GPLv3+',

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
