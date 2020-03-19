from setuptools import setup, find_packages
from cmake_setuptools import *

__version__ = 'v1.1.0'

setup(
    name='tomopackets',
    version=__version__,
    author='Jan-Willem Buurlage',
    author_email='janwillembuurlage@gmail.com',
    url='https://github.com/cicwi/tomopackets',
    description='ZeroMQ based streaming for RT tomography',
    long_description='',
    ext_modules=[CMakeExtension('py_tomop')],
    cmdclass={'build_ext': CMakeBuildExt},
    packages=find_packages(include=['tomop']),
    zip_safe=False,
)
