tar cvfz puuro.tar.gz \
`find . -iname "*.puuro" -or -iname "*.cpp" -or -iname "*.hpp" -or \
        -iname "SConstruct" -or -iname "*.c" -or -iname "*.h" -or \
        -iname "*.html" -or -iname "*.js" -or -iname "*.yy"`
