#!/bin/sh

#
# USAGE:
#
# -- postgis-cvs.tar.gz 
# sh make_dist.sh
#
# -- postgis-1.1.0.tar.gz 
# sh make_dist.sh 1.1.0
#
# NOTE: will not work prior to 1.1.0
#
#

tag=HEAD
version=cvs

if [ -n "$1" ]; then
	version="$1"
	version=`echo $version | sed 's/RC/-rc/'`
	tag=pgis_`echo "$version" | sed 's/\./_/g'`
fi

outdir="postgis-$version"
package="postgis-$version.tar.gz"

if [ -d "$outdir" ]; then
	echo "Output directory $outdir already exist"
	exit 1
fi

echo "Exporting tag $tag"
cvs export -r "$tag" -d "$outdir" postgis 
if [ $? -gt 0 ]; then
	exit 1
fi

# remove .cvsignore, make_dist.sh and HOWTO_RELEASE
echo "Removing .cvsignore and make_dist.sh files"
find "$outdir" -name .cvsignore -exec rm {} \;
rm -f "$outdir"/make_dist.sh "$outdir"/HOWTO_RELEASE

# generating configure script
echo "Running autogen.sh; ./configure"
owd="$PWD"
cd "$outdir"
./autogen.sh
./configure
cd "$owd"

# generating documentation
echo "Generating documentation"
owd="$PWD"
cd "$outdir"/doc
sleep 2 # wait some time to have 'make' recognize it needs to build html
make 
if [ $? -gt 0 ]; then
	exit 1
fi
make clean # won't drop the html dir
cd "$owd"

## generating parser
#echo "Generating parser"
#owd="$PWD"
#cd "$outdir"/lwgeom
#make lex.yy.c
#if [ $? -gt 0 ]; then
#	exit 1
#fi
#cd "$owd"

# Run make distclean
echo "Running make distclean"
owd="$PWD"
cd "$outdir"
make distclean
cd "$owd"

echo "Generating $package file"
tar czf "$package" "$outdir"

#echo "Cleaning up"
#rm -Rf "$outdir"

