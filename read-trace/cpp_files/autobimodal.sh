traces=('gcc_trace.txt' 'jpeg_trace.txt' 'perl_trace.txt')

for t in "${traces[@]}"
do
	for i in {7..20}
	do
		./sim bimodal $i traces/$t > ${t}_bimodal_${i}_${t}
		
	done
done

