cat files.txt | xargs rm
python setup.py clean --all && \
CFLAGS='-Wall -O0 -g' python setup.py install --force --record files.txt && \
python setup.py build_sphinx && \
rst2html.py README.rst doc/build/html/README.html && \
#sphinx-build -b latex doc/source doc/build/latex/ && \
#cd doc/build/latex && make && cd - && \
python setup.py test
