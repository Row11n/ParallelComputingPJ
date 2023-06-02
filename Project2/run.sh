scale_list=("1000" "5000" "10000" "100000" "1000000")
num_thread=("4" "8" "16" "24" "32")

for ((i=0;i<5;i+=1))
do
    for ((j=0;j<5;j+=1))
    do
    mpiexec -n ${num_thread[i]} python main.py -s ${scale_list[j]}
    done
done    