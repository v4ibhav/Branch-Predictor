traces=('gcc_trace.txt')

for t in "${traces[@]}"
do
	for i in {7..20}
	do
		for n in $( seq 0 $i )
		do
			./sim gshare $i $n traces/$t > gshare_${i}_${n}_${t}
		done
	done

done

