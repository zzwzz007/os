#include <iostream>
#include <pthread.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

#define ALL_TICKET 20

union semun
{
    int val;               /* value for SETVAL */
    struct semid_ds *buf;  /* buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* array for GETALL, SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
};

void P(int semid, int index);
void V(int semid, int index);

int ticket = 0;

int semid = -1;
pthread_t t1, t2, t3;

union semun arg0;

void *subp(void *);

int main()
{
    semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);

    arg0.val = 1;
    semctl(semid, 0, SETVAL, arg0);
    int num1 = 1, num2 = 2, num3 = 3;
    pthread_create(&t1, NULL, subp, &num1);
    pthread_create(&t2, NULL, subp, &num2);
    pthread_create(&t3, NULL, subp, &num3);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    semctl(semid, 0, IPC_RMID);
    pthread_exit(NULL);
}

void *subp(void *n)
{
    int sell = 0;
    int id = *(int *)n;
    srand((unsigned)time(NULL));
    while (1)
    {
        sleep(1 + rand() % 3);
        P(semid, 0);
        if (ticket < ALL_TICKET)
        {
            ticket++;
            cout << "id " << id << " sell ticket " << ticket << endl;
            V(semid, 0);
            sell++;
        }
        else
        {
            V(semid, 0);
            break;
        }
    }
    cout << "id " << id << " totally sell " << sell << endl;

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