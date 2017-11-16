:: Build for Python 2
call %userprofile%\AppData\Local\Continuum\anaconda2\Scripts\activate
python setup.py build bdist_wheel
conda config --set anaconda_upload no
python setup.py build bdist_conda

:: Build for Python 2
call %userprofile%\AppData\Local\Continuum\anaconda3\Scripts\activate
python setup.py build bdist_wheel
conda config --set anaconda_upload no
python setup.py build bdist_conda