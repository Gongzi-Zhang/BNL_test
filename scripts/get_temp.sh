#!/bin/bash

output=temp.csv
for run in {1576..2235}; do
    serviceFile=$(getFile "Run${run}_ServiceInfo.txt")
    if [ -f "$serviceFile" ]; then
	awk "NR > 1 {for(i=2; i<=NF; i++) { sum[i] += \$i; count[i]++;} } END{printf \"${run} \"; for (i=2; i<=NF; i++) {printf \"%f \", sum[i]/count[i]} print;}" $serviceFile >> $output
    fi
done
