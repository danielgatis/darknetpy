import sys
import subprocess
import shutil
import re
import tempfile
import os
import zipfile

try:
  import urllib.request as retriver
except:
  import urllib as retriver

from setuptools import setup

try:
    from setuptools_rust import RustExtension, build_ext
except ImportError:
    errno = subprocess.call([sys.executable, '-m', 'pip', 'install', 'setuptools-rust'])

    if errno:
        print('Please install setuptools-rust package')
        raise SystemExit(errno)
    else:
        from setuptools_rust import RustExtension, build_ext


class BuildExtCommand(build_ext):
    def run(self):
        tempdir = tempfile.gettempdir()
        darknet_url = 'https://github.com/pjreddie/darknet/archive/61c9d02ec461e30d55762ec7669d6a1d3c356fb2.zip'
        darknet_zip_file = os.path.join(tempdir, 'darknet.zip')
        darknet_root = os.path.join(tempdir, 'darknet-61c9d02ec461e30d55762ec7669d6a1d3c356fb2')
        makefile = os.path.join(darknet_root, 'Makefile')

        os.environ['DARKNET_ROOT'] = darknet_root

        retriver.urlretrieve(darknet_url, darknet_zip_file)

        with zipfile.ZipFile(darknet_zip_file, 'r') as zip_ref:
            zip_ref.extractall(tempdir)

        if (os.environ.get('GPU', None)):
            sed('GPU=0', 'GPU=1', makefile, count=1)

        if (os.environ.get('CUDNN', None)):
            sed('CUDNN=0', 'CUDNN=1', makefile, count=1)

        if (os.environ.get('OPENCV', None)):
          sed('OPENCV=0', 'OPENCV=1', makefile, count=1)

        if (os.environ.get('OPENMP', None)):
            sed('OPENMP=0', 'OPENMP=1', makefile, count=1)

        if (os.environ.get('DEBUG', None)):
            sed('DEBUG=0', 'DEBUG=1', makefile, count=1)

        process = subprocess.Popen('make', cwd=darknet_root, shell=True)
        process.wait()

        self.include_dirs.append(tempdir)
        self.include_dirs.append(os.path.join(darknet_root, 'include'))

        self.library_dirs.append(darknet_root)
        self.libraries.append('darknet')

        build_ext.run(self)


def readme():
    with open('README.rst') as f:
        return f.read()


def sed(pattern, replace, source, dest=None, count=0):
    fin = open(source, 'r', encoding='latin-1')
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


setup_requires = ['setuptools-rust', 'wheel']
install_requires = []


setup(
    name='darknetpy',
    version='4.0',
    long_description=readme(),
    author='Daniel Gatis Carrazzoni',
    author_email='danielgatis@gmail.com',
    url='https://github.com/danielgatis/darknetpy',
    license='BSD License',
    platforms=['Linux'],
    classifiers=[
        'Intended Audience :: Developers',
        'Programming Language :: Python',
        'Programming Language :: Python :: 3'
    ],
    packages=['darknetpy'],
    rust_extensions=[RustExtension('darknetpy.darknetpy')],
    install_requires=install_requires,
    setup_requires=setup_requires,
    include_package_data=True,
    zip_safe=False,
    cmdclass=dict(build_ext=BuildExtCommand),
)
