#include <iostream>
#include <sys/types.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string>
#include <fstream>
#include <time.h>

#define BUF_LEN 128

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

int semid = -1;
union semun arg_w, arg_r;

const string read_file = "read.txt";
const string write_file = "write.txt";

int main()
{
    int shmid = shmget(IPC_PRIVATE, sizeof(char) * BUF_LEN, IPC_CREAT | 0666);
    semid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);

    arg_r.val = BUF_LEN; //read
    arg_w.val = 0;       //write

    //origin set
    semctl(semid, 0, SETVAL, arg_r);
    semctl(semid, 1, SETVAL, arg_w);

    ifstream fin;
    ofstream fout;

    int file_size = 0;
    fin.open(read_file.c_str());
    if (fin.is_open())
    {
        //get file size
        fin.seekg(0, ios::end);
        file_size = fin.tellg();
        fin.close();
        cout << "size:" << file_size << endl;
    }
    clock_t start, end;
    start = clock();
    int read_buf = fork();
    if (read_buf == 0)
    {
        //son of read
        char c;
        char *shmaddr = (char *)shmat(shmid, NULL, 0);
        fin.open(read_file.c_str());
        for (int i = 0; i < file_size; i++)
        {
            fin.get(c);
            P(semid, 0);
            *(shmaddr + (i % BUF_LEN)) = c;
            V(semid, 1);
        }
        cout << "read end! \n";
    }
    else
    {
        int write_buf = fork();
        if (write_buf == 0)
        {
            //son of write
            char c;
            char *shmaddr = (char *)shmat(shmid, NULL, 0);
            fout.open(write_file.c_str());
            for (int i = 0; i < file_size; i++)
            {
                P(semid, 1);
                c = *(shmaddr + (i % BUF_LEN));
                fout << c;
                V(semid, 0);
            }
            cout << "write end! \n";
        }
        else
        {
            //parent
            waitpid(read_buf, NULL, 0);
            waitpid(write_buf, NULL, 0);
            end = clock();
            cout << "pass time:" << (double)(end - start) / CLOCKS_PER_SEC << endl;
            semctl(semid, 0, IPC_RMID);
            shmctl(shmid, IPC_RMID, NULL);
            cout << "parent end!\n";
            exit(0);
        }
    }
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
