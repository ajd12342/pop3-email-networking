#!/bin/bash

#ins=IITDH
ins=IITB

if [ "$ins" == "IITDH" ]; then
    course=cs348
    ROLLS_FILE=iitdh-rolls-only.csv
else
    course=cs224
    ROLLS_FILE=iitb-rolls-only.csv
fi

while read R
do
    if [ -d "submissions/$ins.$R" ]; then
	echo "Getting tgz for $R"
	dirname="$R-socket-lab2"
	cp -ra "submissions/$ins.$R" "/tmp/$dirname"
	pushd /tmp
	tar zcf "${dirname}.tgz" "$dirname"
	sudo chmod 644 "${dirname}.tgz"
	sudo chown www-data:www-data "${dirname}.tgz"
	sudo mv "${dirname}.tgz" /var/www/html/palantir/$course/$R/lab-marks/
	rm -rf "/tmp/$dirname"
	popd
	echo "done"
    else
	echo "No dir submissions/$ins.$R"
    fi
done < $ROLLS_FILE

