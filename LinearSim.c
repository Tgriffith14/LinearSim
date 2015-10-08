#include "Report.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#define THREE 3
#define FOUR 4
#define FIVE 5
#define MAX 50
#define MAXPIPES 97

static char *Cells, *LeftValue = "L0", *RightValue = "R0", *SimTime;
static int Ndx = 0, Flag, ReportFds[2], ArrNdx, Few = 0, Many = 0,
 Status, ReportArr[MAX] = {0}, Pid, PidArr[MAX], Pipes[MAXPIPES][2],
 PipeNdx = 0, Check[FOUR] = {0}, ExitFlag = 0;
static char *Value, IdNum[FOUR] = "DN", *Cmd[FIVE * 2] = {0};
static char *ReadFds[FOUR] = {"I5", "I7", "I9", "I11"};
static char *WriteFds[FOUR] = {"O6", "O8", "O10", "O12"};

void ArgsCheck(char **argv) {

   Flag = 1;
   while (*++argv) {
      if (**argv == 'C' && !Check[0]) {
         Cells = *argv + 1;
         Check[0] = 1;
      }
      else if (**argv == 'S' && !Check[1]) {
         SimTime = *argv;
         Check[1] = 1;
      }
      else if (**argv == 'L' && !Check[2]) {
         LeftValue = *argv;
         Check[2] = 1;
      }
      else if (**argv == 'R' && !Check[THREE]) {
         RightValue = *argv;
         Check[THREE] = 1;
      }
   }

   if (Check[0] == 0 || Check[1] == 0) {
      fprintf(stderr, "Usage: LinearSim C S L R (in any order)\n");
      exit(1);
   }
   if ((!strcmp(Cells, "1") && Check[THREE] == 1) || atoi(Cells) < 1) {
      fprintf(stderr, "Usage: LinearSim C S L R (in any order)\n");
      exit(1);
   }
}

void PipeFin() {
   while (ArrNdx < Status && Flag > 1) {
      dup2(Pipes[PipeNdx + ArrNdx][0], ArrNdx + Ndx);
      close(Pipes[PipeNdx + ArrNdx][0]);
      dup2(Pipes[PipeNdx + ArrNdx][1], ArrNdx + Ndx + 1);
      close(Pipes[PipeNdx + ArrNdx][1]);
      Ndx++;
      ArrNdx++;
   }

   ArrNdx = FIVE;
   if (Status == 1) {
      Cmd[ArrNdx++] = WriteFds[0];
      close(FIVE);
   }
   else if (Flag == 1 && THREE == atoi(Cells)) {
      Cmd[ArrNdx++] = ReadFds[0];
      close(FIVE + 1);
      Cmd[ArrNdx++] = ReadFds[1];
      close(FIVE + THREE);
   }
   else if (Status == THREE) {
      Cmd[ArrNdx++] = ReadFds[0];
      close(FIVE + 1);
      Cmd[ArrNdx++] = WriteFds[1];
      close(FIVE + 2);
      Cmd[ArrNdx++] = ReadFds[2];
      close(FIVE * 2);
   }
   else {
      Cmd[ArrNdx++] = ReadFds[0];
      close(FIVE + 1);
      Cmd[ArrNdx++] = WriteFds[1];
      close(FIVE + 2);
      Cmd[ArrNdx++] = WriteFds[2];
      close(FIVE + FOUR);
      Cmd[ArrNdx++] = ReadFds[THREE];
      close(FIVE * 2 + 2);
   }
   Cmd[ArrNdx] = NULL;
   execvp("./Cell", Cmd);
}

void ForkLoop() {
   if (!(Pid = fork())) {
      
      sprintf(IdNum + 1, "%d", Ndx);
      
      if (Ndx == 0 && Check[2] == 1) {
         Value = LeftValue;
         Value[0] = 'V';
      }
      else if (Ndx == atoi(Cells) - 1 && Check[THREE] == 1) {
         Value = RightValue;
         Value[0] = 'V';
      }
      else {
         Value = "V0";      
      }
      
      Cmd[2] = IdNum;
      Cmd[THREE] = Value;
      Cmd[FOUR] = "O4";      
      if (!Ndx) {
         Status = 1;
      }
      else if (Ndx == atoi(Cells) - 1) {
         Status = 1;
         PipeNdx -= 1;
      }
      else if (Ndx == 1 || Ndx == atoi(Cells) - 2) {
         Status = THREE;
      } 
      else {
         Status = FOUR;
      }
      ArrNdx = 0;
      Flag = Ndx;
      Ndx = FIVE;
      PipeNdx -= Status - 1;
      
      PipeFin();
   }
}

void PipeLoop() {
   pipe(ReportFds);
   Cmd[0] = "Cell";
   Cmd[1] = SimTime;

   while (Ndx < atoi(Cells)) {
      if (!Ndx) {
         pipe(Pipes[PipeNdx]);
      }
      else if (Ndx == atoi(Cells) - 2) {
         pipe(Pipes[PipeNdx]);
      }
      else if (Ndx != atoi(Cells) - 1) {
         pipe(Pipes[PipeNdx++]);
         pipe(Pipes[PipeNdx]);
      }
      
      ForkLoop();

      PidArr[Ndx] = Pid;
      PipeNdx++;
      Ndx++;
   }
}

void Results() {
   Report report;

   close(ReportFds[1]);
   Ndx = 0;
   while (!close(Ndx + FIVE)) {
      Ndx++;
   }

   while (read(ReportFds[0], &report, sizeof(Report))) {
      fprintf(stdout, "Result from %d, step %d: %.3lf\n", report.id,
       report.step, report.value);
      ReportArr[report.id]++;
   }
   close(ReportFds[0]);
}

void ReportCheck() {
   Ndx = 0;
   while (Ndx < atoi(Cells)) {
      waitpid(PidArr[Ndx], &Status, 0);
      if (EXIT_FAILURE == WEXITSTATUS(Status)) {
         fprintf(stderr, "Error: Child %d exited with %d\n", Ndx,
          EXIT_FAILURE);
         ExitFlag = 1;
      }
      Ndx++;
   }
   Ndx = 0;
   while (Ndx < atoi(Cells)) {
      if (ReportArr[Ndx] < atoi(SimTime + 1) + 1) {
         Few++;
      }
      if (ReportArr[Ndx] > atoi(SimTime + 1) + 1) {
         Many++;
      }
      Ndx++;
   }

   if (Few) {
      fprintf(stderr, "Error: %d cells reported too few reports\n", Few);
      ExitFlag = 1;
   }
   if (Many) {
      fprintf(stderr, "Error: %d cells reported too many reports\n", Many);
      ExitFlag = 1;
   }
   if (atoi(SimTime + 1) < 0 || ExitFlag) {
      exit(1);
   }
}

int main(int argc, char **argv) {

   ArgsCheck(argv);
   PipeLoop();
   Results();
   ReportCheck();

   return 0;
}
