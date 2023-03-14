# HeavyLocker for Heavy Hitter Detection

## How to run

Suppose you've already cloned the repository.

You just need:

```
$ make 
$ ./main ./XX.data (memory) (HT) (d) (LT) (hash-num) (metrics-1, metrics-2...)
```

`XX.data` is a dataset, memory is the memory usage (unit is MB), HT is the heavy hitter threshold, d is the number of cells per bucket, LT is the lock threshold, hash-num is the number of hash functions (i.e., candidate buckets), metrics-1... is the metrics you want.

For example, we can type

```
$ ./main ./caida.dat 0.1 0.0001 4 0.5 1 ARE Precision F1
```

## Output format

Our program will print the Precision, Recall, F1-score, ARE of HeavyLocker and other five sketches based on your input.
