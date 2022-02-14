#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"

sem_t *sem1;
sem_t *sem2;

// T5.4 begin -> T5.4 end -> T7.3 begin -> T7.3 end -> T5.3 begin -> T5.3 end

// T7.1
void* thread_function_7_1(void* arg) {
    int th_id = (int)(size_t)arg;
    info(BEGIN, 7, th_id);
    info(END, 7, th_id);
    pthread_exit(0);
    return (void*)123;
}

// threads for process p7
void* thread_function_7(void* arg) {
    int th_id = (int)(size_t)arg;
    if (th_id == 3) {
        // wait for T5.4 to end before starting T7.3
        sem_wait(sem1);
        info(BEGIN, 7, th_id);
        info(END, 7, th_id);
        // at the end of T7.3, notify T5.3 that it may start
        sem_post(sem2);
    } else {
        info(BEGIN, 7, th_id);
        if(th_id == 2) {
            pthread_t tid = 0;
            int a = 1;
            // create T7.1 from T7.2
            int error = pthread_create(&tid, NULL, thread_function_7_1, (void*)(size_t)a);
            if (error != 0){
                perror("Cannot create thread\n");
                exit(2);
            }
            pthread_join(tid, NULL);
        }

        info(END, 7, th_id);
    }
    pthread_exit(0);
    return (void*)123;
}

int nr_threads_in_critical_reg = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// threads for process p8
void* thread_function_8(void* arg) {
    int th_id = (int)(size_t)arg;

    pthread_mutex_lock(&lock);
    // At any time, at most 5 threads of process P8 could be running simultaneously.
    while (nr_threads_in_critical_reg > 4) {
        pthread_cond_wait(&cond, &lock);
    }
    nr_threads_in_critical_reg++;
    pthread_mutex_unlock(&lock);

    info(BEGIN, 8, th_id);

    if (th_id == 10) {
        if (nr_threads_in_critical_reg == 5) {
            info(END, 8, th_id);
            pthread_mutex_lock(&lock);
            nr_threads_in_critical_reg--;
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&lock);

            pthread_exit(0);
            return (void*)123;
        } 
    }

    info(END, 8, th_id);

    pthread_mutex_lock(&lock);
    nr_threads_in_critical_reg--;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);
    pthread_exit(0);
    return (void*)123;
}

// threads for process p5
void* thread_function_5(void* arg) {
    int th_id = (int)(size_t)arg;

    if (th_id == 4) {
        info(BEGIN, 5, th_id);
        info(END, 5, th_id);
        // at the end of T5.4, notify T7.3 that it may start
        sem_post(sem1);
    } else if (th_id == 3) {
        // wait for T7.3 to end before starting T5.3
        sem_wait(sem2);
        info(BEGIN, 5, th_id);
        info(END, 5, th_id);
    } else {
        info(BEGIN, 5, th_id);
        info(END, 5, th_id);
    }

    pthread_exit(0);
    return (void*)123;
}


int main(){
    // tester initialization
    // only one time in the main process
    init();
    
    // inform tester about (main) process' start
    info(BEGIN, 1, 0);
    
    sem_unlink("sem_one_grama_malina");
    sem_unlink("sem_two_grama_malina");

    sem1 = sem_open("sem_one_grama_malina", O_CREAT, 0644, 0);
    sem2 = sem_open("sem_two_grama_malina", O_CREAT, 0644, 0);

    // create process tree
    pid_t p2, p3, p4, p5, p6, p7, p8;
    
    // P4
    p4 = fork();
    if(p4 == 0) {
        info(BEGIN, 4, 0);
        info(END, 4, 0);
        exit(0);
    } else if (p4 == -1) {
        perror("Cannot create process\n");
        exit(2);
    }

    // P2
    p2 = fork();
    if(p2 == 0) {
        info(BEGIN, 2, 0);

        // P3
        p3 = fork();
        if (p3 == 0) {
            info(BEGIN, 3, 0);

            // P8
            p8 = fork();
            if(p8 == 0) {
                info(BEGIN, 8, 0);
                    int n = 39;
                    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * n);
                    for (int i = 0; i < n; i++){
                        pthread_t tid = 0;
                        int error = pthread_create(&tid, NULL, thread_function_8, (void*)(size_t)(i+1));
                        if (error != 0){
                            perror("Cannot create threads\n");
                            exit(1);
                        }
                        threads[i] = tid;
                    }
                    for(int i = 0; i < n; i++){
                        pthread_join(threads[i], NULL);
                    }
                info(END, 8, 0);
                exit(0);
            } else if (p8 == -1) {
                perror("Cannot create process\n");
                exit(2);
            }

            // wait for P8
            wait(NULL);
            info(END, 3, 0);
            exit(0);
        } else if (p3 == -1) {
            perror("Cannot create process\n");
            exit(2);
        }

        // P5
        p5 = fork();
        if(p5 == 0) {
            info(BEGIN, 5, 0);
            int n = 4;
            pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * n);
            for (int i = 0; i < n; i++){
                pthread_t tid = 0;
                int error = pthread_create(&tid, NULL, thread_function_5, (void*)(size_t)(i+1));
                if (error != 0){
                    perror("Cannot create threads\n");
                    exit(1);
                }
                threads[i] = tid;
            }
            for(int i = 0; i < n; i++){
                pthread_join(threads[i], NULL);
            }
            info(END, 5, 0);
            exit(0);
        } else if (p5 == -1) {
            perror("Cannot create process\n");
            exit(2);
        }

        // P6
        p6 = fork();
        if(p6 == 0) {
            info(BEGIN, 6, 0);
            info(END, 6, 0);
            exit(0);
        } else if (p6 == -1) {
            perror("Cannot create process\n");
            exit(2);
        }

        // P7
        p7 = fork();
        if(p7 == 0) {
            info(BEGIN, 7, 0);
            int n = 4;
            pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * n);
            for (int i = 1; i < n; i++){
                pthread_t tid = 0;
                int error = pthread_create(&tid, NULL, thread_function_7, (void*)(size_t)(i+1));
                if (error != 0){
                    perror("Cannot create threads\n");
                    exit(1);
                }
                threads[i] = tid;
            }
            for(int i = 0; i < n; i++){
                pthread_join(threads[i], NULL);
            }
            info(END, 7, 0);
            exit(0);
        } else if (p7 == -1) {
            perror("Cannot create process\n");
            exit(2);
        }

        // wait for P3
        wait(NULL);
        // wait for P5
        wait(NULL);
        // wait for P6
        wait(NULL);
        // wait for P7
        wait(NULL);
        info(END, 2, 0);
        exit(0);

    } else if (p2 == -1) {
        perror("Cannot create process\n");
        exit(2);
    }

    // wait for P2
    wait(NULL);
    // wait for P4
    wait(NULL);
    sem_close(sem1);
    sem_close(sem2);
    sem_unlink("sem_one_grama_malina");
    sem_unlink("sem_two_grama_malina");
    // inform the tester about (main) process’ termination
    info(END, 1, 0);
    
    return 0;
}