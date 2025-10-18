#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

int hours = 0, minutes = 0, seconds = 0;
unsigned long total_time = 0;

timer_t timer_id;
struct itimerspec new_value;
sigset_t mask, oldmask;

void timer_handler(int sig);
void help_page();

int main(int argc, char* argv[]){
  if (argc == 1){
    //bad, i need more args
    //err msg
    fprintf(stderr, "Needs atleast one argument\n");
    fprintf(stderr, "Try 'timer --help' for more information\n");
    exit(0);
  }

  //all good
  if (argc > 1 && !strcmp(argv[1], "--help")){
    help_page();
  }
  //parse input
  for(int i = 1; i < argc; i++){
    int len = strlen(argv[i]);
    char *arg = argv[i];

    if (len < 2) {
      fprintf(stderr, "Error: '%s' too short (need format: 5h, 30m, 45s)\n", arg);
      help_page();
      exit(1);
    }

    for (size_t j = 0; j < len - 1; j++) {
      if (!isdigit(arg[j])) {
        fprintf(stderr, "Error: '%s' contains non-digit '%c'\n", arg, arg[j]);
        help_page();
        exit(1);
      }
    }

    char unit = arg[len - 1];
    int value = atoi(arg);

    if (value < 0) {
      fprintf(stderr, "Error: negative value in '%s'\n", arg);
      exit(1);
    }

    switch (unit) {
      case 'h':
        hours += value;  // Note: += allows "2h 3h" → 5h
        break;
      case 'm':
        minutes += value;
        break;
      case 's':
        seconds += value;
        break;
      default:
        fprintf(stderr, "Error: unknown unit '%c' in '%s'\n", unit, arg);
        fprintf(stderr, "Valid units: h (hours), m (minutes), s (seconds)\n");
        help_page();
        exit(1);
    }

  }
  //convert all to seconds 
  total_time = seconds + (minutes * 60) + (hours * 3600);
  //set a timer

  if(sigemptyset(&mask)){
    perror("sigemptyset");
    exit(1);
  }
  if(sigaddset(&mask, SIGALRM)){
    perror("sigaddset");
    exit(1);
  }
  if(sigprocmask(SIG_BLOCK, &mask, &oldmask)){
    perror("sigprocmask");
    exit(1);
  }

  if(signal(SIGALRM, timer_handler)){  
    perror("signal");
    exit(1);
  }

  if(!fork()){

    if(timer_create(CLOCK_MONOTONIC, NULL, &timer_id)){
      perror("timer_create");
      exit(1);
    }

    //arm timer
    new_value.it_value.tv_sec = total_time;
    new_value.it_value.tv_nsec = 0;
    new_value.it_interval.tv_sec = 0;
    new_value.it_interval.tv_nsec = 0;
    if(timer_settime(timer_id, 0, &new_value, NULL)){
      perror("timer_settime");
      exit(1);
    }

    if(sigsuspend(&oldmask)){
      perror("sigsuspend");
      exit(1);
    }
  } else {
    exit(0);
  }


  //show a progress bar
  return 0;
}

void timer_handler(int sig){
  if(system("notify-send 'Timer Finished!' 'Your countdown timer has completed.' --icon=/home/asephan/Projects/timer/stopwatch.png -u normal")){ //change icon location to its absoluute path
    perror("system");
    exit(1);
  }
  printf("\a\a\a");
  if(timer_delete(timer_id)){
    perror("timer_delete");
    exit(1);
  }
  if(sigprocmask(SIG_UNBLOCK, &mask, NULL)){
    exit(1);
  }
  exit(0);
}

void help_page(){
  printf("Usage: timer [hours]h [minutes]m [seconds]s\n");
  printf("Sets a timer for time given\n");
  printf("Example: timer 1h 25m 5s\n");
  printf("         timer 25m 5s\n");
  printf("         timer 1h 5s\n");
  exit(0);
}
