#!/usr/bin/python

from setuptools import setup
from distutils.extension import Extension
from Pyrex.Distutils import build_ext

setup(
    name="PyHesiod",
    version="0.2.4",
    description="PyHesiod - Python bindings for the Heisod naming library",
    author="Evan Broder",
    author_email="broder@mit.edu",
    url="http://ebroder.net/code/PyHesiod",
    license="MIT",
    requires=['Pyrex'],
    py_modules=['hesiod'],
    ext_modules=[
        Extension("_hesiod",
                  ["_hesiod.pyx"],
                  libraries=["hesiod"])
        ],
    cmdclass= {"build_ext": build_ext}
)
