from darknetpy.detector import Detector

detector = Detector('/Users/daniel/Workspace/darknet',
                    '/Users/daniel/Workspace/darknet/cfg/coco.data',
                    '/Users/daniel/Workspace/darknet/cfg/yolo.cfg',
                    '/Users/daniel/Workspace/darknet/yolo.weights')

results = detector.detect('/Users/daniel/Workspace/darknet/data/dog.jpg')

print(results)
