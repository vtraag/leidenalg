REM Build for Python 2.7 32 bits
c:\Python27\python.exe setup_ms.py --verbose bdist_msi --target-version=2.7
if %errorlevel% neq 0 exit /b %errorlevel%

REM Build for Python 2.7 64 bits
c:\Python27-x64\python.exe setup_ms.py --verbose bdist_msi --target-version=2.7
if %errorlevel% neq 0 exit /b %errorlevel%

REM Build for Python 3.4 32 bits
c:\Python34\python.exe setup_ms.py --verbose bdist_msi --target-version=3.4
if %errorlevel% neq 0 exit /b %errorlevel%

REM Build for Python 3.4 64 bits
c:\Python34-x64\python.exe setup_ms.py --verbose bdist_msi --target-version=3.4
if %errorlevel% neq 0 exit /b %errorlevel%
