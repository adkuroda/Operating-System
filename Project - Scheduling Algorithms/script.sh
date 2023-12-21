
echo FCFS > output.txt 
make fcfs
./fcfs schedule.txt >> output.txt
echo -e "\n" >> output.txt
echo SJF >> output.txt
make sjf
./sjf schedule.txt >> output.txt
echo -e "\n" >> output.txt
echo PRIORITY >> output.txt
make priority
./priority schedule.txt >> output.txt
echo -e "\n" >> output.txt
echo RR >> output.txt
make rr
./rr schedule.txt >> output.txt
echo -e "\n" >> output.txt
echo PRIORITY-RR >> output.txt
make priority_rr
./priority_rr schedule.txt >> output.txt
echo -e "\n" >> output.txt
