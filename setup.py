#!usr/bin/env python

import os
import platform
import sys

###########################################################################

# Check Python's version info and exit early if it is too old
if sys.version_info < (3, 7):
    print("This module requires Python >= 3.7")
    sys.exit(0)

###########################################################################

from setuptools import setup, Extension

try:
    from wheel.bdist_wheel import bdist_wheel
except ImportError:
    bdist_wheel = None

import glob

if bdist_wheel is not None:
    class bdist_wheel_abi3(bdist_wheel):
        def get_tag(self):
            python, abi, plat = super().get_tag()
            if python.startswith("cp"):
                # on CPython, our wheels are abi3 and compatible back to 3.9
                return "cp39", "abi3", plat

            return python, abi, plat
else:
    bdist_wheel_abi3 = None

# We are going to build an abi3 wheel if we are at least on CPython 3.9.
# This is because the C code contains conditionals for CPython 3.7 and
# 3.8 so we cannot use an abi3 wheel built with CPython 3.7 or 3.8 on
# CPython 3.9
should_build_abi3_wheel = (
    False and # Disable abi3 wheels for now
    bdist_wheel_abi3 and
    platform.python_implementation() == "CPython" and
    sys.version_info >= (3, 9)
)

# Define the extension
macros = []
if should_build_abi3_wheel:
    macros.append(("Py_LIMITED_API", "0x03090000"))
leiden_ext = Extension('leidenalg._c_leiden',
                    sources = glob.glob(os.path.join('src', 'leidenalg', '*.cpp')),
                    py_limited_api=should_build_abi3_wheel,
                    define_macros=macros,
                    libraries = ['libleidenalg', 'igraph'],
                    include_dirs=['include', 'build-deps/install/include'],
                    library_dirs=['build-deps/install/lib']);

description = """
Leiden is a general algorithm for methods of community detection in large networks.

Please refer to the `documentation <http://leidenalg.readthedocs.io/en/latest>`_
for more details.

The source code of this package is hosted at `GitHub <https://github.com/vtraag/leidenalg>`_.
Issues and bug reports are welcome at https://github.com/vtraag/leidenalg/issues.
"""

cmdclass = {}

if should_build_abi3_wheel:
    cmdclass["bdist_wheel"] = bdist_wheel_abi3

options =  dict(
  name = 'leidenalg',
  use_scm_version={
        'write_to': 'src/leidenalg/version.py',
  },
  setup_requires=['setuptools_scm'],
  url = 'https://github.com/vtraag/leidenalg',
  description = 'Leiden is a general algorithm for methods of community detection in large networks.',
  long_description=description,
  license = 'GPLv3+',
  author = 'V.A. Traag',
  author_email = 'vincent@traag.net',
  ext_modules = [leiden_ext],
  test_suite = 'tests',
  package_dir = {'leidenalg': os.path.join('src', 'leidenalg')},
  packages = ['leidenalg'],

  provides = ['leidenalg'],
  python_requires=">=3.7",
  install_requires = ['igraph >= 0.10.0,< 0.11'],
  platforms="ALL",
  keywords=[
    'graph',
    'network',
    'community detection',
    'clustering'
    ],
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
    ],
    cmdclass=cmdclass,
)

setup(**options)
