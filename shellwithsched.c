//CST-315
//Project 3 Short-Term Process Scheduler
//Improved shell with better file system navigation and background and foreground process scheduling
//To compile: gcc shellwithsched.c
//To run: ./a.out
/*Available commands in the shell:
exit
pwd
cd
help function is currently broken, this will cause errors but the shell will still work
>
*/
//batch mode is functioning
//this shell uses foreground-background scheduling making it simple priority based: every process will enter the forground list for execution and wait there a small period of time, after this time if the process has not completed then it will enter the background waiting list and will only execute when the foreground list is empty.
#include <stdlib.h>
#include <unistd.h> 
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>

void myPrint(char *msg)
{
    write(STDOUT_FILENO, msg, strlen(msg));
}
char error[30] = "\nAn error has occurred\n"; // prints error message

//testing if input is greater than the max possible length
#define MAX 514
//To parse in input from cmd line, returns number of commands that can be used
int cmd_help(char* c, char** cmds){
    char* help = malloc(sizeof(char));
    char* locptr = malloc(sizeof(char));
    locptr = NULL;
    help = strtok_r(c, " \n\a\t\r\v\f", &locptr);
    
    int counter = 0;
    while (help != NULL) {
        char* temp = malloc(sizeof(char));
	temp = help;
	cmds[counter] = temp;
        counter++;
        help = strtok_r(NULL, " \n\a\t\r\v\f", &locptr);
    }
    cmds[counter] = NULL;
    counter--;
    return counter;   
    free(help);
    free(locptr);
}
//determines if a character c is whitespace
int special_char(char c){
int isnt_special_char = 1;
if ((c == '\n') || (c == '\t') || (c == '\v') || (c == '\f') || (c == '\r') || (c == ' ')) {
isnt_special_char = 0;   
}
return isnt_special_char;
}

int special_word(char* c){
int isnt_special_word = 1;
int len = strlen(c);
for (int i = 0; i < len; i++) {
if (special_char(c[i])) {
isnt_special_word = 0;
}
}
return isnt_special_word;
}

//for the exit command
void exit_cmd(char** args, int len){
    if (len>1) {
        myPrint(error); //error if there are too many arg with exit cmd
    }
    exit(0);
}
//for the pwd command
void pwd_cmd(char** args, int len){
    char* temp = malloc(sizeof(char)*200);
    if (len>1) {
        myPrint(error); //error if there are too many arg with the exit cmd
    }
    getcwd(temp, 200); // to change pwd to temp and print new pwd
    free(temp);
}

//for the cd command
void cd_cmd(char** args, int len){
    //testing
    if (len == 1) {                        
        if (chdir(getenv("HOME")) == -1) { // if no dir then go home
            myPrint(error);                // chdir error
        }
    }else if (len == 2) {                  //if dir 1, go to dir
        if (chdir(args[1])== -1) {
            myPrint(error);                // chdir error
        }
    }else if (len > 2 || len < 1) {         
        myPrint(error);                    //error if too many arg with cd cmd
    }
}

//for > redirection commands
void redirect_cmd(char** args, int len) {
    //for the first else-if
    char* temp = malloc(sizeof(char));
    temp = NULL;
    char* FILE = malloc(sizeof(char));
    FILE = NULL;
    char* locptr = malloc(sizeof(char));
    locptr = NULL;
    char* redirect_loc = malloc(sizeof(char));
    redirect_loc = strchr(args[len], '>'); //pointer to wherever the location of > is
    int lenhelp = strlen(args[len-1])-1; //helper for final case

    //instances for redirection
    // for command file
    if (args[len][0] == '>') {
    FILE = args[len]+1; //getting the filename
    args[len] = NULL;   //removing after >
    }
    // for command > file
    else if (redirect_loc != NULL) {
    temp = strtok_r(args[len], ">", &locptr);  //get command
    FILE = strtok_r(NULL, ">", &locptr);       //get filename
    args[len] = temp;                          //delete everything after the command
    }
    // for command > file
    else if (args[len-1][0] == '>') {
    FILE = args[len];   //get filename
    args[len] = NULL;   //remove filename
    args[len-1] = NULL; //remove redirection
    }
    // for command> file
    else if (args[len-1][lenhelp]=='>') {
    FILE = args[len];           //get filename
    args[len] = NULL;           //remove filename
    args[len-1][lenhelp] = 0;   //remove redirection
    }
    //else any other format with the > will be invalid
    else {
        myPrint(error);
        exit(0);
    }

    //variables for the second else-if
    int accessable = access(FILE, F_OK); // testing for existance of file
    pid_t id = fork();
    int wait_help; //wait()
    int openablef; //open()
    int redirection; //dup2()
    int executable; // execvp()
    int open_method; //open()
    if (accessable != -1) {
        myPrint(error); //error unaccessable
    } else if (id < 0) {
        myPrint(error); //error for fork()
        exit(0);
    } else if (id == 0) {

      
    open_method = O_WRONLY | O_CREAT; //Write only, and if DNE, create it.

    //for mode for read/writing to be true for users
    mode_t open_permissions = S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH;
    openablef = open(FILE, open_method, open_permissions);
    //tests if the file cannot be opened   
    if (openablef < 0) {
        myPrint(error);
        exit(0);
        }
    //testing for error with dup2() change STDOUT(=1) to openablef   
    redirection = dup2(openablef, 1);
    if (redirection < 0) {
        myPrint(error); 
        exit(0);
        }
	  //tests for execute error
    executable = execvp(args[0], args);
    if (executable == -1) {
        myPrint(error); 
        exit(0);
        }
        close(openablef);
        exit(0);
    } else {
      while(id != wait(&wait_help));
      return; 
    }
      free(temp);
      free(FILE);
      free(locptr);
      free(redirect_loc);
}

  //for any and all other commands that have not been built in
  void other_help(char** cmds){
  pid_t id;
  id = fork();
  int wait_help;  //wait()
  int executable; //execvp()

  // for fork error
  if (id<0) {
  myPrint(error);
  exit(0); //an execute error
  } else if (id == 0) {
	executable = execvp(cmds[0], cmds);
    if(executable == -1){
        myPrint(cmds[0]);
        myPrint(error); 
        exit(0);
        }
    } else { //there are no errors
        //waiting for child process
        while(id != wait(&wait_help));
        return;
    }
}

void choose_helper(char* cmd_buff){
  char* args[200];
  char* locptr = NULL;
  char* temp = NULL;      
  int len;
  int i = 0;
  int j = 0;
  int redirect = 0;
  int advredirect = 0;
  temp = strtok_r(cmd_buff, ";\n", &locptr); //cmds split by ; or \n
  while (temp != NULL) { //go through all commands
      if (!special_char(temp[0])) //disreguard the initial space there
          temp = temp + 1;
  //checking for any redirection be going though each character in cmd      
	len = cmd_help(temp, args); //the number of commands
	if (len >= 0) {
        for (i=0; i<=len; i++) { //checking each word and letter
            for (j=0; args[i][j] != 0; j++) { 
                if(args[i][j] == '>') {
                redirect = redirect + 1;
                if (args[i][j+1] == '+') {
                advredirect++;
                }
            }
      }
  }
//when exit is called	
  if (strstr(args[0], "exit") != NULL) {
      if (redirect) {
        myPrint(error); //error for a redirection with exit
        break;
        }
        exit_cmd(args, len+1);
        }

  //cd called
  else if (strstr(args[0], "cd") != NULL) {
           if (redirect) {
           myPrint(error); //redirection with cd
           break;
           }
           cd_cmd(args, len+1);
        }
  //pwd called
  else if (strstr(args[0], "pwd") != NULL) {
          if (redirect) {
          myPrint(error); //redirection with pwd
          break;
          }
          pwd_cmd(args, len+1);
        }
  //for more than 1 redirects
  else if (redirect > 1) {
           myPrint(error); 
           break;
        }
  //redirection called
  else if(redirect == 1){
          redirect_cmd(args, len);
          break;
          } else {  //else for all other cases repeat/reset for every command
          other_help(args);
        }
        }
        temp = strtok_r(NULL, ";\n", &locptr);
    }
}
//main
int main(int argc, char *argv[]){
  char cmd_buff[MAX] = {'\0'};
  char *pinput = malloc(sizeof(char));
  FILE* f;
  int batchmode = 0;
  int len = 0;
    
  if (argc > 2) { //for too many arguments
      myPrint(error);
      exit(0);
    }

  else if (argc == 2) { //for batch mode
          batchmode = batchmode + 1;
          f = fopen(argv[1], "r");
	if (f == NULL) {
          myPrint(error); //error data file
          exit(0);
        }
    }

  while (5) {
	 if (batchmode) {
       pinput = fgets(cmd_buff, 1100, f);
       if (!pinput) { //if nothing is entered
       exit(0);
    }
  //dealing with any line breaks   
 	len = strlen(cmd_buff);
  char* lastchar = &(cmd_buff[len-1]);
   	 if (strcmp(lastchar, "\n") == 0) {
       	 *lastchar = '\0';
  }

     if (strcmp(cmd_buff, "\n") != 0) { //input is not newline char
        if (!special_word(pinput)) {
        myPrint(cmd_buff);
          if (strlen(cmd_buff) > MAX) { 
          myPrint("\n");              
			    myPrint(error);                       
		    } else {
          myPrint("\n");
          choose_helper(cmd_buff);
          }
      }
  }

    //if not in batch
    } else {
        myPrint("newshell> ");
        pinput = fgets(cmd_buff, 514, stdin); //514 bytes if stdin stored to cmd buffer
        if (!pinput) { //if there is nothing entered
          exit(0);
        }
          len = strlen(cmd_buff);
            if (len < 2 || len > 514) {
                myPrint("\n");
                myPrint(error);
          } else {
                choose_helper(cmd_buff);
            }
        }
    }
    if(batchmode)
    fclose(f);
    return 0; 
    free(pinput);
}