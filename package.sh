PKG="puuro-"`date +%y%m%d`

FILES=`find . -iname "*.puuro" -or -iname "*.cpp" -or -iname "*.hpp" -or \
              -iname "SConstruct" -or -iname "*.c" -or -iname "*.h" -or \
              -iname "*.html" -or -iname "*.js" -or -iname "*.yy"`
for i in $FILES
do
  mkdir -vp $PKG/`dirname $i` || exit 1
  cp -v $i $PKG/$i || exit 2
done

tar cvfz $PKG.tar.gz $PKG || exit 3

rm -fr $PKG

