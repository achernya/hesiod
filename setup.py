#!/usr/bin/python

from setuptools import setup
from distutils.extension import Extension
from Pyrex.Distutils import build_ext

setup(
    name="PyHesiod",
    version="0.1.1",
    description="PyHesiod - Python bindings for the Heisod naming library",
    author="Evan Broder",
    author_email="broder@mit.edu",
    license="MIT",
    ext_modules=[
        Extension("hesiod",
                  ["hesiod.pyx"],
                  libraries=["hesiod"])
        ],
    cmdclass= {"build_ext": build_ext}
)
