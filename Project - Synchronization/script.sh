g++ driver.cpp Shop.cpp -o sleepingBarbers -lpthread
echo -e 'CMD INPUT: 1 1 10 1000' > out.txt
./sleepingBarbers 1 1 10 1000 >> out.txt 
echo -e '\n CMD INPUT: 3 1 10 1000' >> out.txt
./sleepingBarbers 3 1 10 1000 >> out.txt 


echo -e '\n change in chair number of seats' >> out.txt
for i in {1..90}
do
    echo 'number of seats =' $i >> out.txt
    ./sleepingBarbers 1 $i 200 1000 | grep 'did' >> out.txt
done


echo -e '\n change in number of barbers' >> out.txt
for i in {1..6}
do
    echo 'number of seats =' $i >> out.txt
    ./sleepingBarbers $i 0 200 1000 | grep 'did' >> out.txt
done

rm sleepingBarbers