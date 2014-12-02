
for imgpth in img/*
do
	name=`basename $imgpth .img`
	p2="dump/img/"$name".dump"
	xxd $imgpth > $p2
done

for imgpth in tst/*
do
	name=`basename $imgpth .img`
	p2="dump/tst/"$name".dump"
	xxd $imgpth > $p2
done
