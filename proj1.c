#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>   // for srand
#include <unistd.h> // contian type def pid_t

// defines
pid_t p1, p2, R;
FILE *fp1, *fp2;

void increment_bigScore(char y[20]);
void calculate_score(char x[BUFSIZ]);
void print_on_Child1();
void print_on_Child2();
int guess();

char *tok_namefile[2], *token_scores[2];
char score[20];    // store the value from ref R
int gobal_pipe[2]; //create global pipe to talk when call function or from other files

//defines the scores variables
int bigScore1 = 0, bigScore2 = 0;
int pid1, pid2, pidR;

int roundR = 0; // for number of round we play

int main()
{

    srand(time(NULL)); // must initalize once
    //creat the messages and give size of it as the buffer size to avoid any issue with size when send and receive message
    static char message_p1[BUFSIZ] = "P1 start generate random number";
    static char message_p2[BUFSIZ] = "P2 start generate random number";
    static char message_pp1[BUFSIZ] = "File child1.txt is ready\n";
    static char message_pp2[BUFSIZ] = "File child2.txt is ready\n";
    static char file_names[BUFSIZ] = "child1.txt-child2.txt";
    int pipe_parent[2]; // for connection pipe ( 0 read  1 write ) --> p1 parent write --> read
    int pipe_child[2];  // for connection pipe ( 0 read  1 write ) --> p2 child write --> read
    int pipe_R[2];      // pipe to communication between parent and R

    // calls functions
    void signal_SIGUSR1_catcher(int);
    void signal_SIGINT_catcher(int);

    // check the pipe
    if (pipe(pipe_parent) == -1)
    {
        printf("Unable to create pipe for parent, try again \n");
        return 1;
    }
    if (pipe(pipe_child) == -1)
    {
        printf("Unable to create pipe with children, try again \n");
        return 1;
    }
    if (pipe(pipe_R) == -1)
    {
        printf("Unable to create pipe with R, try again \n");
        return 1;
    }
    if (pipe(gobal_pipe) == -1)
    {
        printf("Unable to create pipe with gobal_pipe, try again \n");
        return 1;
    }
    // Create childrens
    p1 = fork();
    //child 1
    if (p1 < 0)
    { // check fork
        printf("Failure fork, exit bye\n");
        exit(-1);
    }
    else if (p1 == 0)
    {
        printf("Child 1 id: %d and the parent: %d\n", getpid(), getppid());
        if (sigset(SIGUSR1, signal_SIGUSR1_catcher) == -1)
        { //SIG_ERR : error return (-1)
            perror("Sigset can not set SIGUSR1\n");
            exit(SIGUSR1);
        }

        pid1 = getpid();//store id for p1 

        while (1)
        {
            // pipe
            close(pipe_child[0]);  // close child read from pipe 2
            close(pipe_parent[1]); // close write

            read(pipe_parent[0], message_p1, BUFSIZ); // read from parent
            printf("\nThe message form parent:%s\n", message_p1);
            print_on_Child1(); // print in file

            write(pipe_child[1], message_pp1, BUFSIZ);
        }
    }
    else
    {
        //child 2
        p2 = fork();
        if (p2 == 0)
        {   
            printf("Child 2 id: %d and the parent: %d\n", getpid(), getppid());
            if (sigset(SIGUSR1, signal_SIGUSR1_catcher) == -1)
            { //SIG_ERR : error return (-1)
                perror("Sigset can not set SIGUSR1\n");
                exit(SIGUSR1);
            }

            pid2 = getpid(); //store the id for p2

            while (1)
            {
                close(pipe_child[0]);  // close child read from pipe 2
                close(pipe_parent[1]); // close write
                read(pipe_parent[0], message_p2, BUFSIZ);
                printf("\nThe message form parent:%s\n", message_p2);
                print_on_Child2(); // print in file

                write(pipe_child[1], message_pp2, BUFSIZ);
            }
        }
        else
        {
            //Third Child which is the referee process
            R = fork();
            if (R == 0)
            {
                printf("R (referee) id: %d and the parent: %d\n", getpid(), getppid());
                if (sigset(SIGUSR1, signal_SIGUSR1_catcher) == -1)
                { //SIG_ERR : error return (-1)
                    perror("Sigset can not set SIGUSR1\n");
                    exit(SIGUSR1);
                }

                pidR = getpid();//store the ip for R

                while (1)
                {

                    pause();
                    close(pipe_R[1]);
                    read(pipe_R[0], file_names, BUFSIZ);
                    printf("\nProcess R Receive message from parent:%s\n",file_names);
                    close(gobal_pipe[0]); // close read from global pipe since we only want to read;
                    calculate_score(file_names); // calculate the score every round
                     
                }
              
            }
            else
            {   
                // ** PARENT **

                if ((sigset(SIGINT, signal_SIGINT_catcher) || sigset(SIGQUIT, signal_SIGINT_catcher)) == -1)
                {
                    perror("Sigset can not set SIGINT or SIGOUT");
                    exit(1);
                }

                if (sigset(SIGUSR1, signal_SIGUSR1_catcher) == -1)
                { //SIG_ERR : error return (-1)
                    perror("Sigset can not set SIGUSR1\n");
                    exit(SIGUSR1);
                }

                while (1)
                {
                    roundR++;
                    printf("\nRound: %d\n", roundR);

                    write(pipe_parent[1], message_p1, BUFSIZ); // write to the pipe
                    write(pipe_parent[1], message_p2, BUFSIZ); // write to the pipe
                    sleep(1);
                    printf("\n%s",message_pp1);
                    read(pipe_child[0], message_pp1, BUFSIZ); // read from pipe

                    printf("%s",message_pp2);
                    read(pipe_child[0], message_pp2, BUFSIZ); // read from pipe

                    kill(pidR, SIGUSR1);

                    write(pipe_R[1], file_names, BUFSIZ); // write on R

                    read(gobal_pipe[0], score, 20);
                    printf("From R the score send to parent is: %s\n", score);

                    increment_bigScore(score);// function that update on bigScore 1 and bigScore2

                    if (bigScore1 >= 50 || bigScore2 >= 50)
                    {
                        break;
                    }
                }

                 //Compare on bigScore to detemin which is the winner
                if (bigScore1 > bigScore2)
                {
                   printf(" \n**process P1 is the winner!**\n");
                }
                else if (bigScore1 < bigScore2)
                {
                    printf("\n**process P2 is the winner!**\n");
                }
                else
                {
                    printf("\n***process P1 & P2 both are the winner!***\n ");
                }

                kill(pid1, SIGINT);
                kill(pid2, SIGINT);
                //kill(pidR, SIGINT);
                sleep(1);
            }
        }
    }

    return (0);
}

// functions to handel the signal
void signal_SIGUSR1_catcher(int the_sig)
{
    //printf("\nThe child has received SIGUSR1 from parent.\n");
}

void signal_SIGINT_catcher(int the_sig)
{
    printf("\n Parent KIll his child (P1,P2 & R),Program has finished!\n");
    exit(0);
}

// function to generate random number between 1 and 100
int guess()
{
    int num, min = 1, max = 100;
    num = (rand() % (max - min + 1)) + min;
    return num;
}
// function to make p1 print random numbers  of file child1
// here if file exsit or not not a problem it will make override on it
void print_on_Child1()
{
    fp1 = fopen("child1.txt", "w"); // open folder for read only
  if (fp1 == NULL)
    {
        printf("Error read from child1.txt");
        exit(0);
    }
     // loop to print 10 line of numbers
    for (int i = 0; i < 10; i++)
    {
        int g = guess();
        fprintf(fp1, "%d\n", g);
    }
    fclose(fp1);
}

// function to make p2 print random numbers of file child2
void print_on_Child2()
{
    fp2 = fopen("child2.txt", "w"); // open file for write only
     // loop to print 10 line of numbers
  if (fp2 == NULL)
    {
        printf("Error read from child1.txt");
        exit(0);
    }
    for (int i = 0; i < 10; i++)
    {
        int gu = guess();
        fprintf(fp2, "%d\n", gu);
    }
    fclose(fp2);
}

// function to calculate the score every round
void calculate_score(char x[BUFSIZ])
{
    int i = 0, n1, n2;          // defines varaibles
    int score1 = 0, score2 = 0; // for calculate score by ref R

    // take the message and spilt by -
    char *token_f = strtok(x, "-"); // make token -

    while (token_f != NULL)
    { // store the values form token into an array

        tok_namefile[i++] = token_f;
        token_f = strtok(NULL, "/");
    }

    // open files to read from it.
    fp1 = fopen(tok_namefile[0], "r");
    fp2 = fopen(tok_namefile[1], "r");
    // check if files are empty or not
    if (fp1 == NULL)
    {
        printf("Error read from child1.txt");
        exit(0);
    }
    if (fp2 == NULL)
    {
        printf("Error read from child2.txt");
        exit(1);
    }
    else
    { // if files not empty compare between it and return the score
        while (fscanf(fp1, "%d", &n1) != EOF && fscanf(fp2, "%d", &n2) != EOF)
        {
            if (n1 > n2)
            {
                score1++;
            }
            else if (n2 > n1)
            {
                score2++;
            }
            if (n1 == n2)
            {
                continue;
            }
        }
        // close files
        fclose(fp1);
        fclose(fp2);
        //remove files
       // remove("child1.txt");  
       // remove("child2.txt"); 

        sprintf(score, "%d-%d", score1, score2); // convert to char
        printf("Score from files is:%s\n", score);
    }

    write(gobal_pipe[1], score, 20); // write on gobal_pipe from R
}

void increment_bigScore(char y[20])
{
    int l = 0;
    // take the message and spilt by -
    char *tok_score = strtok(y, "-");
    while (tok_score != NULL)
    {
        token_scores[l++] = tok_score;
        tok_score = strtok(NULL, "/");
    }
    // convert form char to int
    int s1 = atoi(token_scores[0]);
    int s2 = atoi(token_scores[1]);

    // update for score in round
    bigScore1 = bigScore1 + s1;
    bigScore2 = bigScore2 + s2;
    printf("The score for child1 (bigSCore1) is: %d\n", bigScore1);
    printf("The score for child2 (bigSCore2) is: %d\n", bigScore2);
}