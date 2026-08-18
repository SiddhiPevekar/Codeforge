# Plot the results
cat throughput_data.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Overall Throughtput vs No. of clients" -X "No. of Clients" -Y "Throughput(Requests per second)" -r 0.25 > ./throughput.png

cat response_time_data.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Avg. Response Time vs No. of clients " -X "No. of Clients" -Y "Avg. Response Time(in s)" -r 0.25 > ./response_time.png

cat request_rate.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Avg. Request rate vs No. of clients " -X "No. of Clients" -Y "Avg. Requests(per s)" -r 0.25 > ./request_rate.png

cat goodput_data.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Overall Goodput vs No. of clients " -X "No. of Clients" -Y "Goodput(Requests per second)" -r 0.25 > ./goodput_data.png

cat timeout_rate.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Avg. Timeout Rate vs No. of clients " -X "No. of Clients" -Y "Avg. Timeouts(per s)" -r 0.25 > ./timeout_rate.png

cat error_rate.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Avg. Error Rate vs No. of clients " -X "No. of Clients" -Y "Avg. Number of Errors(per s)" -r 0.25 > ./error_rate.png

cat cpu_util.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Avg. CPU Utilization vs No. of clients " -X "No. of Clients" -Y "Avg. CPU Utilization(in percentage)" -r 0.25 > ./cpu_util.png

cat nlwp_data.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Avg. NLWP vs No. of clients " -X "No. of Clients" -Y "Avg. Number of Threads(at any instance)" -r 0.25 > ./nlwp.png

echo "Done"
