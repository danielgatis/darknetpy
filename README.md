# Darknetpy
darknetpy is a simple binding for darknet's yolo detector.

# Installation
```bash
pip install darknetpy
```

## Advanced options:
```bash
GPU=1 pip install darknetpy
```
to build with CUDA to accelerate by using GPU (CUDA should be in /use/local/cuda).

```bash
CUDNN=1 pip install darknetpy
```
to build with cuDNN to accelerate training by using GPU (cuDNN should be in /usr/local/cudnn).

```bash
OPENCV=1 pip install darknetpy
```
to build with OpenCV.

```bash
OPENMP=1 pip install darknetpy
```
to build with OpenMP support to accelerate Yolo by using multi-core CPU.

# Usage

In example.py
```python
from darknetpy.detector import Detector
from darknetpy.utils import suppress_stdout_stderr

with suppress_stdout_stderr():
    detector = Detector('<absolute-path-to>/darknet',
                        '<absolute-path-to>/darknet/cfg/coco.data',
                        '<absolute-path-to>/darknet/cfg/yolo.cfg',
                        '<absolute-path-to>/darknet/yolo.weights')

results = detector.detect('<absolute-path-to>/darknet/data/dog.jpg')

print(results)
```

Runing
```bash
python example.py
```

Result
```bash
[{'right': 194, 'bottom': 353, 'top': 264, 'class': 'dog', 'prob': 0.8198755383491516, 'left': 71}]
```

