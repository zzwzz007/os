#include <iostream>
#include <pthread.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

union semun
{
    int val;               /* value for SETVAL */
    struct semid_ds *buf;  /* buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* array for GETALL, SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
};

void P(int semid, int index);
void V(int semid, int index);

int a = 0;

int semid = -1;
pthread_t t1, t2, t3;

union semun arg0, arg1;

void *subp1(void *);
void *subp2(void *);
void *subp3(void *);

int main()
{
    srand((unsigned)time(NULL));
    semid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);

    arg0.val = 0;
    arg1.val = 1;
    semctl(semid, 0, SETVAL, arg0); //print
    semctl(semid, 1, SETVAL, arg1); //cal

    int num1 = 1, num2 = 2, num3 = 3;
    pthread_create(&t1, NULL, subp1, &num1);
    pthread_create(&t2, NULL, subp2, &num2);
    pthread_create(&t3, NULL, subp3, &num3);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    pthread_exit(NULL);
}

void *subp1(void *n)
{
    //calculate
    for (int i = 1; i <= 100; i++)
    {
        sleep(1);
        P(semid, 1);
        a += i;
        cout << "add num " << i << endl;
        V(semid, 0);
    }

    return 0;
}

void *subp2(void *n)
{
    //2n
    while (1)
    {
        if (a == 5050)
            break;
        sleep(1);
        P(semid, 0);
        if (a % 2 == 0)
        {
            sleep(1);
            cout << "thread 1 print num " << a << endl;
            V(semid, 1);
        }
        V(semid, 0);
    }

    return 0;
}

void *subp3(void *n)
{
    //2n+1
    while (1)
    {
        if (a == 5050)
            break;
        sleep(1);
        P(semid, 0);
        if (a % 2 != 0)
        {
            sleep(1);
            cout << "thread 2 print num " << a << endl;
            V(semid, 1);
        }
        V(semid, 0);
    }

    return 0;
}

void P(int semid, int index)
{
    struct sembuf sem;
    sem.sem_num = index;
    sem.sem_op = -1;
    sem.sem_flg = 0;
    semop(semid, &sem, 1);
    return;
}

void V(int semid, int index)
{
    struct sembuf sem;
    sem.sem_num = index;
    sem.sem_op = 1;
    sem.sem_flg = 0;
    semop(semid, &sem, 1);
    return;
}