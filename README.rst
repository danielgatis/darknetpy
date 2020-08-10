=========
Darknetpy
=========

|Downloads| |DownloadsMonth| |DownloadsWeek|

.. |Downloads| image:: https://pepy.tech/badge/darknetpy
   :target: https://pepy.tech/project/darknetpy

.. |DownloadsMonth| image:: https://pepy.tech/badge/darknetpy/month
   :target: https://pepy.tech/project/darknetpy/month

.. |DownloadsWeek| image:: https://pepy.tech/badge/darknetpy/week
   :target: https://pepy.tech/project/darknetpy/week
   
Darknetpy is a simple binding for darknet's yolo (v4) detector.

.. image:: https://raw.githubusercontent.com/danielgatis/darknetpy/master/example/example.png

Installation
============

Install it from pypi

::

    curl https://sh.rustup.rs -sSf | sh

::

    rustup default nightly

::

    pip install darknetpy

Install a pre-built binary

::

    pip install https://github.com/danielgatis/darknetpy/raw/master/dist/darknetpy-4.1-cp36-cp36m-linux_x86_64.whl

Advanced options (only for pypi installation)
---------------------------------------------
::

    GPU=1 pip install darknetpy

to build with CUDA to accelerate by using GPU (CUDA should be in /use/local/cuda).

::

    CUDNN=1 pip install darknetpy

to build with cuDNN to accelerate training by using GPU (cuDNN should be in /usr/local/cudnn).

::

    OPENCV=1 pip install darknetpy

to build with OpenCV.

::

    OPENMP=1 pip install darknetpy

to build with OpenMP support to accelerate Yolo by using multi-core CPU.

Usage
=====

In example.py::

    from darknetpy.detector import Detector

    detector = Detector('<absolute-path-to>/darknet/cfg/coco.data',
                        '<absolute-path-to>/darknet/cfg/yolo.cfg',
                        '<absolute-path-to>/darknet/yolo.weights')

    results = detector.detect('<absolute-path-to>/darknet/data/dog.jpg')

    print(results)

Runing::

    python example.py


Result::

    [{'right': 194, 'bottom': 353, 'top': 264, 'class': 'dog', 'prob': 0.8198755383491516, 'left': 71}]

Build
=====

On the project root directory

::

    docker pull hoshizora/manylinux1-clang_x86_64

::

    docker run --rm -v `pwd`:/io hoshizora/manylinux1-clang_x86_64 /io/build-wheels.sh
