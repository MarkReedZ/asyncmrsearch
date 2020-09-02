
from setuptools import setup, Extension, find_packages

m1 = Extension(
    'asyncmrsearch.internals',
     sources = [
      './src/asyncmrsearch/internals/city.c',
      #'./src/asyncmrsearch/internals/plist.c',
      './src/asyncmrsearch/internals/module.c',
      './src/asyncmrsearch/internals/mrserver.c',
      './src/asyncmrsearch/internals/mrclient.c',
      './src/asyncmrsearch/internals/mrprotocol.c',
     ],
     include_dirs = ['./src/asyncmrsearch/internals'],
     extra_compile_args = ['-msse4.2', '-mavx2', '-mbmi2', '-Wunused-variable','-std=c99','-Wno-discarded-qualifiers', '-Wno-unused-variable','-Wno-unused-function'],
     extra_link_args = [],
     define_macros = [('DEBUG_PRINT',1)]
)

setup(
    name='asyncmrsearch',
    version="0.2",
    description='Python asyncio mrmrsearch client',
    long_description='Asyncio based Python client for mrmrsearch',
    classifiers=[
        'Intended Audience :: Developers',
        'Programming Language :: Python',
        'Programming Language :: Python :: 3',
        ],
    url='https://github.com/MarkReedZ/asyncmrsearch',
    author='Mark Reed',
    author_email='mark@untilfluent.com',
    license='MIT License',
    ext_modules = [m1],
    package_dir={'':'src'},
    packages=find_packages('src'),# + ['prof'],
    zip_safe=True,
)
