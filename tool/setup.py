from distutils.core import setup, Extension

module1 = Extension('sere',
                    define_macros = [('MAJOR_VERSION', '0'),
                                     ('MINOR_VERSION', '9')],
                    include_dirs = ['/home/dima/work/sere/build/install/include',
                                    '/usr/local/include'],
                    libraries = ['sere_shared'],
                    library_dirs = ['/home/dima/work/sere/build/install/lib',
                                    '/usr/local/lib'],
                    sources = ['seremodule.c'])

setup (name = 'Sere',
       version = '0.9',
       description = 'Python binding to SERE library',
       author = 'Dmitry Kulagin',
       author_email = 'dmitry.kulagin@gmail.com',
       url = 'https://github.com/craft095/sere',
       long_description = '''
Python binding to SERE library.
''',
       ext_modules = [module1])
