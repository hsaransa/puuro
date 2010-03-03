import glob

env = Environment()

refcount = int(ARGUMENTS.get('refcount', 1))
gc = int(ARGUMENTS.get('gc', 1))

env.Append(CPPFLAGS=['-g', '-Wall', '-W'])

if not refcount:
  env.Append(CPPDEFINES=[('NO_REFCOUNT')])

if not gc:
  env.Append(CPPDEFINES=[('NO_GC')])

src = []
src += glob.glob('src/*.cpp')
src.sort()
src = ['src/grammar.yy'] + src

env.Program('puuroi', src)
