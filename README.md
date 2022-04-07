# Real-Time-Guessing-Game-Simulation

We would like to create a multi-processing application that simulates a guessing game
between 2 processes using signals and pipes. A parent process will create 2 children
processes and 1 referee process. We’ll call the first child process P1 and the second child
process P2. We will refer to the referee process by the symbol R.
The behavior of the whole system should be as follows:
  1. Upon creation, the children P1 and P2 will be sensitive to signal SIGUSR1. The parent process will be sensitive to both signals SIGINT and SIGQUIT.
  2. The parent process will ask P1 and P2 to pick each 10 random integer numbers in the range [1 . . . 100].
  3. Once the 10 random integer numbers are picked, P1 will write the numbers in a file called child1.txt where each number will be in a separate line in the file.
  4. P2 will write the integer numbers it picked in a file called child2.txt where each number will be in a separate line in the file.
  5. Once file child1.txt is ready, P1 will inform the parent process that its file is ready to be processed.
  6. Once file child2.txt is ready, P2 will inform the parent process that its file is ready to be processed.
  7. Upon being informed by P1 and P2, the parent process will inform the referee pro- cess R to decide on the winner. It does that by sending a message to R that contains the files child1.txt and child2.txt separated by a dash character (e.g. child1.txt-child2.txt).
  8. The referee process R opens the 2 files and compares the integer numbers line by line. If a number picked by P1 in a particular line is higher than the respective number picked by P2, score1 is incremented. If a number picked by P2 in a particular line is higher than the respective number picked by P1, score2 is incremented. Otherwise, score1 and score2 remain unchanged. The above behavior is done for every line in the 2 files. Afterwards, R process deletes the files child1.txt and child2.txt.
  9. R process will send to the parent process the scores of P1 and P2 processes separated by a dash (e.g. 5-3).
  10. The parent process parses the received message from process R and increments global counters associated with P1 and P2 respectively using the score values. We’ll call the global counters of P1 and P2 bigScore1 and bigScore2 respectively. This ends the current round.
  11. Go to step 2. above for a new round unless either bigScore1 or bigScore2 counters has reached 50.
  12. The parent process declares the winner of the game and the number of needed rounds then kills P1, P2, R and then exits. If both counters bigScore1 and bigScore2 reach 50 at the same time, then both P1 and P2 are winners of the game.
