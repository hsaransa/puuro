# scons file for Puuro

STD2_DIR = 'std2'

import glob, os

env = Environment()

#env['YACC'] = 'byacc'
env['YACCFLAGS'] = ['-d']

env.Append(CPPFLAGS=['-g', '-Wall', '-W'])

# Options

refcount = int(ARGUMENTS.get('refcount', 1))
gc = int(ARGUMENTS.get('gc', 1))

if not refcount:
  env.Append(CPPDEFINES=[('NO_REFCOUNT')])

if not gc:
  env.Append(CPPDEFINES=[('NO_GC')])

env.Append(CPPDEFINES=[('LIB_DIR', '"\\"%s/lib\\""' % os.getcwd())])

# Std2

env.Append(CPPPATH=[STD2_DIR + '/include'])
env.Append(LIBPATH=[STD2_DIR])
env.Append(LIBS=['std2'])

# adns

#env.Append(LIBS=['adns'])

# Sources

src = []
src += glob.glob('src/*.cpp')
src.sort()
src = ['src/grammar.yy'] + src

env.Program('puuroi', src)
