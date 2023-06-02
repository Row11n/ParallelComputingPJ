from mpi4py import MPI
import numpy as np
import random
import time
import bisect
import argparse

def get_args_parser():
    parser = argparse.ArgumentParser('agr_parser', add_help=False)
    parser.add_argument('-s', default=10000, type=int)
    return parser

if __name__ == "__main__":
    comm = MPI.COMM_WORLD
    size = comm.Get_size()
    rank = comm.Get_rank()
    if rank == 0:
        parser = argparse.ArgumentParser('parser', parents=[get_args_parser()])
        args = parser.parse_args()
        scale = args.s
        raw_array = [random.randint(0, scale) for _ in range(scale)]
        start_time_s = time.time()
        backup = [i for i in raw_array]
        backup.sort()
        end_time_s = time.time()
        time_s = end_time_s - start_time_s
        start_time_p = time.time()
        # step 1
        group_size = len(raw_array) // size
        data = [raw_array[i:i+group_size] for i in range(0, len(raw_array), group_size)]
        if len(data) % size != 0:
            last_group = data.pop()
            data[-1].extend(last_group)
        # print(len(data))
    else:
        data = None
    data = comm.scatter(data, root=0)
    
    # step 2
    data.sort()
    # print(f'{rank}\n{data}')
    _size = size + 1
    step = len(data) // _size if (len(data) % _size == 0) else len(data) // _size + 1
    sample_pivots = [data[i] for i in range(0, len(data), step)][1:]

    # step 3 
    sample_pivots = comm.gather(sample_pivots, root=0)
    if rank == 0:
        sample_pivots = [i for sublist in sample_pivots for i in sublist]
        sample_pivots.sort()
        pivots = [sample_pivots[i] for i in range(size, len(sample_pivots), size)]
    else:
        pivots = None

    # step 4
    pivots = comm.bcast(pivots, root=0)
    # print(f'{rank}\n{pivots}')
    # print(f'{rank}\n{data}')
    data_slice = []
    last_split = 0
    for i in range(len(pivots)):
        split = pivots[i]
        temp = data[bisect.bisect_left(data, last_split):bisect.bisect_left(data, split)]
        data_slice.append(temp)
        last_split = split
    data_slice.append(data[bisect.bisect_left(data, last_split):])
    while len(data_slice) < size:
        data_slice.append([])
    
    # step 5
    data_exchange = [data_slice[rank]]
    for i in range(size):
        if rank == i:
            for j in range(size):
                if j == rank:
                    continue
                comm.send(data_slice[j], dest=j, tag=i*10+j)
            for j in range(size):
                if j == rank:
                    continue
                data_exchange.append(comm.recv(source=j, tag=j*10+i))
    data_exchange = [i for sublist in data_exchange for i in sublist]
    # print(f'temp{rank}\n{data_exchange}')

    # step 6
    data_exchange.sort()
    data_exchange = comm.gather(data_exchange, root=0)
    if rank == 0:
        data_exchange = [i for sublist in data_exchange for i in sublist]
        # print(f'final{rank}\n{data_exchange}')
        end_time_p = time.time()
        time_p = end_time_p - start_time_p
        # print(time_s)
        # print(time_p)
        print(f'The number of threads: {size}')
        print(f'Speedup: {time_s/time_p}')

