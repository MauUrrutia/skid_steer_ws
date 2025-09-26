from setuptools import find_packages
from setuptools import setup

setup(
    name='skid_steer_localization',
    version='0.0.0',
    packages=find_packages(
        include=('skid_steer_localization', 'skid_steer_localization.*')),
)
