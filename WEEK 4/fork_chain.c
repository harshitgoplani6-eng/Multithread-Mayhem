#include "kernel/types.h"
#include "user/user.h"

void indepth(int n , int Number){
  if(n>=0){
  int parent_pid= getpid() ; 
  int pid = fork();

    if (pid < 0) {
        printf("fork failed\n");
        exit(1);
    }

    if (pid == 0) {
       //child
      int nxt_n = n-1 ;
        indepth ( nxt_n , Number);
        exit(0);
    } else {
        // Parent
      wait(0) ; 
      printf("Depth %d : PID %d , Parent PID %d\n", n , pid, parent_pid);
        
    }
  }
  else return;
}
int main(int argc, char *argv[]) {
  int Number;
  if (argc < 2) {
    Number = 5; 
  }
  else
    Number = atoi(argv[1]); 

  indepth(Number , Number);

  
    printf("I am the leaf! No more children. \n") ; 
    printf(" Total processes in chain: %d\n" , Number );
    exit(0);
}
