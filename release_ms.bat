@echo off

IF "%1%"=="release" (
    ECHO Performing a release...

    :: Build for Python 2
    call %userprofile%\AppData\Local\Continuum\anaconda2\Scripts\activate
    python setup.py build bdist_wheel
    conda config --set anaconda_upload no
    python setup.py build bdist_conda

    :: Build for Python 3
    call %userprofile%\AppData\Local\Continuum\anaconda3\Scripts\activate
    python setup.py build bdist_wheel
    conda config --set anaconda_upload no
    python setup.py build bdist_conda

    :: Upload to PyPi using twine
    python setup.py --fullname > PACKAGE.txt
    SET /p PACKAGE=<PACKAGE.txt
    DEL PACKAGE.txt
    twine upload dist/%PACKAGE%*win*.whl --sign

) ELSE (
    ECHO Performing a dry-run.

    :: Build for Python 2
    call %userprofile%\AppData\Local\Continuum\anaconda2\Scripts\activate
    python setup.py build bdist_wheel
    conda config --set anaconda_upload no
    python setup.py build bdist_conda

    :: Build for Python 3
    call %userprofile%\AppData\Local\Continuum\anaconda3\Scripts\activate
    python setup.py build bdist_wheel
    conda config --set anaconda_upload no
    python setup.py build bdist_conda

    :: Upload to PyPi using twine
    python setup.py --fullname > PACKAGE.txt
    SET /p PACKAGE=<PACKAGE.txt
    DEL PACKAGE.txt
    twine upload dist/%PACKAGE%*win*.whl --sign -r pypitest

)