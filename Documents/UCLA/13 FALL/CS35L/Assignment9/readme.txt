***After-action report*** The difficult part in this homework is that it was intimidated at first not knowing where to start. I understand the basic struture for parallel programing but I have a hard
time figuring out what needs to be inside the data structure for each thread. After carefully reviewing the loop in the main function, I realized that the function is doing calculation in each pixel
individually, meaning that the arguments in one loop doesn't really affect the rest. Therefore, I put the variable scene and the thread information inside the struction and declared everything else
inside the undated thread. The code looks fine but the output was incorrect. I realized that it is because that even though each thread does its calculation separately, the printf at the end of the
second loop is not in order. Hence I declared a global array with dimension width*height*3. Everytime the calculation is complete I stored the variable inside the array but didn't print them out.
After the parallel process is complete and the threads are joined, I then print out the results inside the array in pixel order. No error reported.  The implementation of SRT improves dratically as
the thread number increases.  When the program is run on single thread, the entire process took around 47 seconds. When the thread number is increased to 8, the time went all the away down to only a
little bit more than 10 seconds, demonstrating the efficiency of parallel programming.  
