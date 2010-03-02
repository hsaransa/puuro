import glob

env = Environment()

env.Append(CPPFLAGS=['-g', '-Wall', '-W'])

src = []
src += glob.glob('src/*.cpp')
src.sort()
src = ['src/grammar.yy'] + src

env.Program('puuroi', src)
