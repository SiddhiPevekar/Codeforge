#!/usr/bin/bash

argc=$#
if [ $argc -ne 3 ]
then
echo "Usage : ./loadtest.sh <numClients> <loopNum> <sleepTimeSeconds>"
exit 1
fi

numClients=$1
loopNum=$2
sleepTimeSeconds=$3
ct=$1

for ((i=1;i<=$ct;i++));
do 
 echo "Launching client $i in background"
./client 192.168.93.221:9999 student_code.cpp $2 $3 > "clientoutput$i.txt" &
done

wait

overallthroughput=0
avgResponseTime=0
numberofResponses=0

for ((j = 1; j <= numClients; j++)); do
  avgResponseTime_i=$(grep "Average Response Time:" "clientoutput$j.txt" | cut -d ':' -f 2)
  successfulResponses_i=$(grep "Number of Successful Responses:" "clientoutput$j.txt" | cut -d ':' -f 2)
  throughput_i=$(grep "Throughput:" "clientoutput$j.txt" | cut -d ':' -f 2)
  
  avgResponseTime=$(echo "$avgResponseTime + $avgResponseTime_i * $successfulResponses_i" | bc -l)
  overallthroughput=$(echo "$overallthroughput + $throughput_i" | bc -l)
  numberofResponses=$(echo "$numberofResponses + $successfulResponses_i" | bc -l)
done

avgResponseTime=$(echo "$avgResponseTime / $numberofResponses" | bc -l)


echo "$numClients $overallthroughput" >> throughput_data.txt
echo "$numClients $avgResponseTime" >> response_time_data.txt


rm clientoutput*

