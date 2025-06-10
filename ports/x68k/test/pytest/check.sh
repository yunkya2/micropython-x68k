#!/bin/bash
TYPES=""
for opt in "$@"; do
	case $opt in
	-*)
		;;
	*)
		TYPES="${TYPES} ${opt}"
		;;
	esac
done
if [ -z "${TYPES}" ]; then
	TYPES="basics extmod float import io micropython misc"
fi

DIFFOUT=/dev/null
SHOWOK=true
while getopts "dn" opt;do
	case $opt in
	d)
		DIFFOUT=/dev/tty
		;;
	n)
		SHOWOK=false
		;;
	*)
		echo "Usage: $0 [-d] [-n] [<test type> ...]"
		echo "  -d: show diff output"
		echo "  -n: do not show OK tests"
		exit 1
		;;
	esac
done

for TYPE in ${TYPES}; do
	TESTS=../../../../tests/${TYPE}
	LOG=log-${TYPE}
	SKIP=checkskip.txt

	echo -n "${TYPE}/"
	if [ ! -d ${LOG} ]; then
		echo " (skipped)"
		continue
	else
		echo ""
	fi

	for a in ${LOG}/*.py;do
		n=$(basename -s .py $a)
		found=""
		while read name comment ; do
			if [ "$name" = "${TYPE}/$n" ]; then
				found=$n
				break
			fi
		done < ${SKIP}
		if [ ! "${found}" = "" ]; then
			if ${SHOWOK}; then echo "[IGNORE] $n" ;fi
			continue
		fi

		if [ -f ${TESTS}/$n.py.exp ]; then
			EXP=${TESTS}/$n.py.exp
		else
			(cd ${TESTS}; python3 $n.py) > pytest.log 2>/dev/null
			EXP=pytest.log
		fi
		LOGFILE=${LOG}/$n.py

		if grep SKIP ${LOGFILE} > /dev/null ; then
			if ${SHOWOK}; then echo "[SKIP] $n" ;fi
			rm -f pytest.log
			continue
		fi

		cat ${LOGFILE} | tr -d '\r' | diff - ${EXP} > ${DIFFOUT}
		if [ $? -eq 0 ]; then 
			if ${SHOWOK}; then echo "[ OK ] $n" ;fi
		else
			echo "[ NG ] $n"
		fi
		rm -f pytest.log
	done
	echo ""
done
