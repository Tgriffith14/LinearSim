#include "Report.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>



int main(int argc, char **argv) {
   int i=0, id, value = 5, sims;
   Report report;

   while(*++argv){
      if(**argv == 'D'){
         id = atoi(*argv + 1);
      }
      if(**argv == 'S'){
         sims = atoi(*argv + 1);
      }
   }
   if (id % 3 == 0) {
      sims = 500;
   }
   report.id = id;
   while(i <= sims){
      report.step = i++;
      report.value = value++;
      write(4, &report, sizeof(Report));
   }

   return 0;
}