=========
Darknetpy
=========

darknetpy is a simple binding for darknet's yolo detector.

.. image:: https://raw.githubusercontent.com/danielgatis/darknetpy/master/example/example.png

Installation
============

::

    pip install darknetpy

Advanced options
--------------------
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
====================

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
