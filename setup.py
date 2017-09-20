import os
import re
import shutil
import subprocess
import tempfile
import urllib.request
import zipfile

from setuptools import Extension, find_packages, setup
from setuptools.command.build_ext import build_ext


def readme():
    with open('README.rst') as f:
        return f.read()


def sed(pattern, replace, source, dest=None, count=0):
    fin = open(source, 'r')
    num_replaced = count

    if dest:
        fout = open(dest, 'w')
    else:
        fd, name = tempfile.mkstemp()
        fout = open(name, 'w')

    for line in fin:
        out = re.sub(pattern, replace, line)
        fout.write(out)

        if out != line:
            num_replaced += 1
        if count and num_replaced > count:
            break
    try:
        fout.writelines(fin.readlines())
    except Exception as E:
        raise E

    fin.close()
    fout.close()

    if not dest:
        shutil.move(name, source)


class BuildExtCommand(build_ext):
    def run(self):
        tempdir = tempfile.gettempdir()
        darknet_url = 'https://github.com/pjreddie/darknet/archive/1e729804f61c8627eb257fba8b83f74e04945db7.zip'
        darknet_zip_file = os.path.join(tempdir, 'darknet.zip')
        darknet_unziped = os.path.join(tempdir, 'darknet-1e729804f61c8627eb257fba8b83f74e04945db7')
        darknet_root = os.path.join(tempdir, 'darknet')
        makefile = os.path.join(darknet_root, 'Makefile')

        try:
            if not os.path.exists(darknet_zip_file):
                urllib.request.urlretrieve(darknet_url, darknet_zip_file)

            if not os.path.exists(darknet_unziped):
                with zipfile.ZipFile(darknet_zip_file, 'r') as zip_ref:
                    zip_ref.extractall(tempdir)

                shutil.move(darknet_unziped, darknet_root)

            if (os.environ.get('GPU', None)):
                sed('GPU=0', 'GPU=1', makefile, count=1)

            if (os.environ.get('CUDNN', None)):
                sed('CUDNN=0', 'CUDNN=1', makefile, count=1)

            if (os.environ.get('OPENCV', None)):
                sed('OPENCV=0', 'OPENCV=1', makefile, count=1)

            if (os.environ.get('OPENMP', None)):
                sed('OPENMP=0', 'OPENMP=1', makefile, count=1)

            process = subprocess.Popen('make', cwd=darknet_root, shell=True)
            process.wait()

            os.remove(os.path.join(darknet_root, 'libdarknet.so'))

            self.include_dirs.append(tempdir)
            self.include_dirs.append(os.path.join(darknet_root, 'include'))

            self.library_dirs.append(darknet_root)
            self.libraries.append('darknet')

            super().run()

        finally:
            try:
                os.remove(darknet_zip_file)
                shutil.rmtree(darknet_root, ignore_errors=True)
            except OSError:
                pass


modules = [
    Extension(
        name='darknetpy.detector',
        sources=['extension/detector.c'],
        extra_compile_args=['-fPIC', '-std=c99']
    )
]


setup(
    name='darknetpy',
    version='1.8',
    long_description=readme(),
    author='Daniel Gatis Carrazzoni',
    author_email='danielgatis@gmail.com',
    url='https://github.com/danielgatis/darknetpy',
    license='BSD License',
    platforms=['Linux'],
    classifiers=[
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Operating System :: POSIX',
        'Programming Language :: Python',
        'Programming Language :: Python :: 3'
    ],
    python_requires='>=3',
    packages=find_packages(exclude=['tests.py']),
    ext_modules=modules,
    cmdclass={'build_ext': BuildExtCommand},
    include_package_data=True,
    zip_safe=False
)
