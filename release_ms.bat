@echo off

IF "%1%"=="release" (
    ECHO Performing a release...

    :: Build for Python 3
    call %userprofile%\AppData\Local\Continuum\anaconda3\Scripts\activate
    python setup.py build bdist_wheel --no-download
    conda config --set anaconda_upload yes
    python setup.py build bdist_conda --no-download

    :: Upload to PyPi using twine
    python setup.py --fullname > PACKAGE.txt
    SET /p PACKAGE=<PACKAGE.txt
    DEL PACKAGE.txt
    gpg --detach-sign -a dist\%PACKAGE%-cp36-cp36m-win_amd64.whl
    twine upload dist\%PACKAGE%-*win*.whl  dist\%PACKAGE%-*win*.whl.asc

) ELSE (
    ECHO Performing a dry-run.

    :: Build for Python 3
    call %userprofile%\AppData\Local\Continuum\anaconda3\Scripts\activate
    python setup.py build bdist_wheel --no-download
    conda config --set anaconda_upload no
    python setup.py build bdist_conda --no-download

    :: Upload to PyPi using twine
    python setup.py --fullname > PACKAGE.txt
    SET /p PACKAGE=<PACKAGE.txt
    DEL PACKAGE.txt
    gpg --detach-sign -a dist\%PACKAGE%-cp36-cp36m-win_amd64.whl
    twine upload dist\%PACKAGE%-cp*win*.whl  dist\%PACKAGE%-cp*win*.whl.asc -r pypitest

)