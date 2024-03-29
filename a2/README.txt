Maxwell Huang-Hobbs
c3huangk
CSC369, Assignment 2


== Task 1

 ----------------------------------------------------------------------- 
| Tracefile      | Markerfile        | I   | S/L/R | Inside   | Outside |
|-----------------------------------------------------------------------|
| simpleloop     | simpleloop.marker | 64  | 2574  | 70803    | 52075   |
| matmul         | matmul.marker     | 74  | 1018  | 23654302 | 52486   |
| blocked-100-25 | blocked.marker    | 77  | 1019  | 23956102 | 52618   |
| make-sim       | <null>            | 128 | 150   | 0        | 1149032 |
| grep-freedom   | <null>            | 133 | 136   | 0        | 404846  |
 ----------------------------------------------------------------------- 

The script for task 1 is `analyze.py`, in the /a2/ directory.

Note: for programs we do not modify the source of, `Outside` is the number
of instructions over the entire run of the process.

simpleloop uses a small number of instructions pages and a large number of
data pages. Moreover, it uses  a comparatively small number of memory
operations when compared to matmul and blocked. This makes sense, all of 
simpleloop's instructions are heap memory allocations in sequence.

In contrast, matmul uses a small number of instructions, a moderate number 
of data pages, and executes a  large number of memory accesses.
matmul only performs arithmetic operations on a few matrices, changing
their values in place over the course of many multiplications. The
relatively low ratio of  data pages used / memory accesses performed is to
be expected of a process that mainly changes values 'in place'.

blocked has near-identical access stats to matmul. Both programs are
implementations of different matrix multiplication algorithms, so it
would not be unreasonable to think they are operating on what is effectively
the same data. The only significant difference/ in the number of data pages 
used would come from temporary variables on the stack, as neither
implementation of matrix multiplication seems to make use of large temporary
variables.

Interestingly, make uses  a small number of data and instruction pages, but 
goes through large number of memory accesses. This is in line with expected
behavior, as make itself only performs some small text operations before
forking and calling exec. The behavior of gcc is outside of the scope of
this course, but it is not unreasonable to assume given the size of the input
and output files that gcc would have similar access patterns.


The interesting program (tracefile grep-freedom) chosen for Task 1 was
grep, and the tracefile was generated by grepping for the word `libre` over
the text of the book `Free as in Freedom: Richard Stallman's Crusade for 
Free Software.

Like make, grep uses a small number of pages and executes a large number
of instructions over the course of it's run. Given grep's function (to
scan bodies of text), we can infer from this that it likely loads only a
small subsection of the text into memory at a time, before scanning,
printing results, and then pulling a new block of text in.


== Task 2

 ------------------------------------------------------------------------------ 
| Alg  | m   | Trace      | Hits     | Misses  | Total    | HitRt    | MissRt  |
|------------------------------------------------------------------------------|
| fifo | 50  | simpleloop | 380013   | 2977    | 382990   |  99.2227 |  0.7773 |
| fifo | 100 | simpleloop | 380240   | 2750    | 382990   |  99.2820 |  0.7180 |
| fifo | 150 | simpleloop | 380280   | 2710    | 382990   |  99.2924 |  0.7076 |
| fifo | 200 | simpleloop | 380288   | 2702    | 382990   |  99.2945 |  0.7055 |
|------------------------------------------------------------------------------|
| fifo | 50  | matmul     | 62055545 | 1129703 | 63185248 |  98.2121 |  1.7879 |
| fifo | 100 | matmul     | 62101279 | 1083969 | 63185248 |  98.2845 |  1.7155 |
| fifo | 150 | matmul     | 63150843 | 34405   | 63185248 |  99.9455 |  0.0545 |
| fifo | 200 | matmul     | 63151365 | 33883   | 63185248 |  99.9464 |  0.0536 |
|------------------------------------------------------------------------------|
| fifo | 50  | blocked    | 67869846 | 6458    | 67876304 |  99.9905 |  0.0095 |
| fifo | 100 | blocked    | 67871968 | 4336    | 67876304 |  99.9936 |  0.0064 |
| fifo | 150 | blocked    | 67872062 | 4242    | 67876304 |  99.9938 |  0.0062 |
| fifo | 200 | blocked    | 67873126 | 3178    | 67876304 |  99.9953 |  0.0047 |
 ------------------------------------------------------------------------------ 

 ----------------------------------------------------------------------------- 
| Alg | m   | Trace      | Hits     | Misses  | Total    | HitRt    | MissRt  |
|-----------------------------------------------------------------------------|
| lru | 50  | simpleloop | 380213   | 2777    | 382990   |  99.2749 |  0.7251 |
| lru | 100 | simpleloop | 380312   | 2678    | 382990   |  99.3008 |  0.6992 |
| lru | 150 | simpleloop | 380314   | 2676    | 382990   |  99.3013 |  0.6987 |
| lru | 200 | simpleloop | 380314   | 2676    | 382990   |  99.3013 |  0.6987 |
|-----------------------------------------------------------------------------|
| lru | 50  | matmul     | 62144028 | 1041220 | 63185248 |  98.3521 |  1.6479 |
| lru | 100 | matmul     | 62178803 | 1006445 | 63185248 |  98.4072 |  1.5928 |
| lru | 150 | matmul     | 63152370 | 32878   | 63185248 |  99.9480 |  0.0520 |
| lru | 200 | matmul     | 63152381 | 32867   | 63185248 |  99.9480 |  0.0520 |
|-----------------------------------------------------------------------------|
| lru | 50  | blocked    | 67871123 | 5181    | 67876304 |  99.9924 |  0.0076 |
| lru | 100 | blocked    | 67872494 | 3810    | 67876304 |  99.9944 |  0.0056 |
| lru | 150 | blocked    | 67872    | 3795    | 67876304 |  99.9944 |  0.0056 |
| lru | 200 | blocked    | 67872612 | 3692    | 67876304 |  99.9946 |  0.0054 |
 ----------------------------------------------------------------------------- 


 ------------------------------------------------------------------------------ 
| Alg   | m   | Trace      | Hits      | Misses  | Total    | HitRt   | MissRt |
|------------------------------------------------------------------------------|
| clock | 50  | simpleloop |  380211   | 2779    | 382990   | 99.2744 | 0.7256 |
| clock | 100 | simpleloop |  380311   | 2679    | 382990   | 99.3005 | 0.6995 |
| clock | 150 | simpleloop |  380314   | 2676    | 382990   | 99.3013 | 0.6987 |
| clock | 200 | simpleloop |  380314   | 2676    | 382990   | 99.3013 | 0.6987 |
|------------------------------------------------------------------------------|
| clock | 50  | matmul     |  62144024 | 1041224 | 63185248 | 98.3521 | 1.6479 |
| clock | 100 | matmul     |  62183466 | 1001782 | 63185248 | 98.4145 | 1.5855 |
| clock | 150 | matmul     |  63150595 | 34653   | 63185248 | 99.9452 | 0.0548 |
| clock | 200 | matmul     |  63152368 | 32880   | 63185248 | 99.9480 | 0.0520 |
|------------------------------------------------------------------------------|
| clock | 50  | blocked    |  67870549 | 5755    | 67876304 | 99.9915 | 0.0085 |
| clock | 100 | blocked    |  67872106 | 4198    | 67876304 | 99.9938 | 0.0062 |
| clock | 150 | blocked    |  67872497 | 3807    | 67876304 | 99.9944 | 0.0056 |
| clock | 200 | blocked    |  67873083 | 3221    | 67876304 | 99.9953 | 0.0047 |
 ------------------------------------------------------------------------------ 


 ---------------------------------------------------------------------------- 
| Alg | m   | Trace      | Hits     | Misses | Total    | HitRt    | MissRt  |
|----------------------------------------------------------------------------|
| opt | 50  | simpleloop | 380323   | 2667   | 382990   |  99.3036 |  0.6964 |
| opt | 100 | simpleloop | 380356   | 2634   | 382990   |  99.3123 |  0.6877 |
| opt | 150 | simpleloop | 380357   | 2633   | 382990   |  99.3125 |  0.6875 |
| opt | 200 | simpleloop | 380357   | 2633   | 382990   |  99.3125 |  0.6875 |
|----------------------------------------------------------------------------|
| opt | 50  | matmul     | 62597794 | 587454 | 63185248 |  99.0703 |  0.9297 |
| opt | 100 | matmul     | 63092456 | 92792  | 63185248 |  99.8531 |  0.1469 |
| opt | 150 | matmul     | 63158640 | 26608  | 63185248 |  99.9579 |  0.0421 |
| opt | 200 | matmul     | 63165990 | 19258  | 63185248 |  99.9695 |  0.0305 |
|----------------------------------------------------------------------------|
| opt | 50  | blocked    | 67872582 | 3722   | 67876304 |  99.9945 |  0.0055 |
| opt | 100 | blocked    | 67873285 | 3019   | 67876304 |  99.9956 |  0.0044 |
| opt | 150 | blocked    | 67873775 | 2529   | 67876304 |  99.9963 |  0.0037 |
| opt | 200 | blocked    | 67874025 | 2279   | 67876304 |  99.9966 |  0.0034 |
 ---------------------------------------------------------------------------- 

The above tables were generated with `/a2/sim/maketable.py` and can be 
re-created by running `make tables` in /a2/sim

All of the heuristics implemented for selecting a page for eviction operate 
within the range of 97-99% hit rate, at least on the sample traces given.
The best performing algorithm was, unsurprisingly, opt. However the runtime
of opt is significantly higher than the other algorithms, and implementing it 
to minimalize runtime is much more difficult than implementing any of the
other algorithms. It should also be noted that a real world implementation
of opt is impractical and in most situations impossible, as it requires 
foreknowledge of how a process will access memory before it executes. LRU
operates well, with clock and fifo close behind. In general, the easier to
implement algorithms (clock, fifo) performed worse than the more difficult to
implement ones.

Looking at the hitrate of for lru as it correlates to the size of physical
memory (m), we can see that the hitrate increases as m increases. Because this
is a "perfect lru implementation", capacity misses only happen when there are
more pages referenced than there are slots in physical memory between two
accesses to any given page. Increasing this number of pages in physcial memory
just increases the tolerance before incurring capacity misses, and therefore
reduces the overall number of misses.


== Task 3

The trace for task 3 (/a2/sim/beladytrace) illustrates Belady's anomaly for
20/21; The hitrate for fifo with a physical memory of 20 frames has a higher 
hit rate (8.51%) than that of fifo with a physical memory of 21 frames (6.48%).

The way the trace is structured, both buffers are initially filled with 
addresses labeled 0 through 19. When address 20 is read in, it pushes address 0
of the front of the smaller queue, but leaves it on the larger queue.

Next, the sequence (0, 1) is read in, This pushes 0,1 to the back of the small
queue, while leaving it at the front of the larger queue. This means when 22 is
read in, it pushes 0 off the bottom of the larger queue, so on the following 
sequence of (0..20), the smaller queue has 2 less misses than the larger one,
so there are less capacity misses by the end of the sequence.

