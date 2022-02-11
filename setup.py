from distutils.core import setup, Extension
import sysconfig

def main():
    CFLAGS = ['-g', '-Wall', '-std=c99', '-fopenmp', '-mavx', '-mfma', '-pthread', '-O3']
    LDFLAGS = ['-fopenmp']
    # Use the setup function we imported and set up the modules.
    # You may find this reference helpful: https://docs.python.org/3.6/extending/building.html
    # TODO: YOUR CODE HERE

    module = Extension(name='numc',
                       sources = ['matrix.c', 'numc.c'],
                       include_dirs = ['/data/verif/courses/CS61c/fa20-proj4-starter'],
                       extra_compile_args = CFLAGS,
                       extra_link_args=LDFLAGS)


    setup(name = 'numc',
          version = '1.0',
          description = 'This is numc package from CS61C proj4',
          ext_modules = [module]
          )
if __name__ == "__main__":
    main()
