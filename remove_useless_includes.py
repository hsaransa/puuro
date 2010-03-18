import os, glob, sys

def write_lines(fn, lines):
    f = open(fn, 'w')
    s = ''
    for l in lines:
        s += l + '\n'
    f.write(s)
    f.close()

def main():
    if os.system('scons'):
        print "stuff doesn't compile"
        return

    rap = ""

    src = sys.argv

    for fn in src:
        orig_file = open(fn).read()
        lines = orig_file.splitlines()

        cands = []
        for row, j in enumerate(lines):
            # Don't remove prdefs.hpp include nor lines that have comments
            if 'prdefs.hpp' in j or '//' in j or '/*' in j:
                continue
            if j.startswith('#include') or j.startswith('#   include'):
                cands.append(row) 

        rm = []

        for r in cands:
            old = lines[r]
            lines[r] = '// ' + old

            write_lines(fn, lines)

            if os.system('scons'):
                lines[r] = old
            else:
                rm.append(r)

        open(fn, 'w').write(orig_file)

        if rm:
            rap += 'removing from ' + fn + '\n'
            for i in rm:
                rap += '  ' + lines[i] + '\n'
            lines = [l for ln, l in enumerate(lines) if not ln in rm]

            write_lines(fn, lines)

            if os.system('scons'):
                write_lines(fn, 'error.log')
                open(fn, 'w').write(orig_file)
                raise "oops, I broke something while in " + fn

    print 'DONE'
    print rap

if __name__ == '__main__':
    main()
