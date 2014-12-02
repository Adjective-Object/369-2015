for dump in img/*
do
	name=`basename $dump .img`
	imgd="dump/img/"$name".dump"
	tstd="dump/tst/"$name".dump"
	out="diffs/"$name".diff"
	diff -y --suppress-common-lines $imgd $tstd > $out
done
