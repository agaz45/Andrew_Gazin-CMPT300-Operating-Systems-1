#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

//defining a boolean type
typedef enum {false, true} bool;

//Handles the command line
void testinput(bool* iflag, bool* lflag, bool* Rflag, char *commands){
    //amount of arguments
    int count = strlen(commands);
    int n=1;

    //sets appropriate flags/options to true
    while (n<count && n<4){
	if(commands[n] == 'l'){
	    *lflag = true;
	}
	else  if(commands[n] == 'i'){
	    *iflag = true;
	}
	else if (commands[n] == 'R'){
	    *Rflag = true;
	}
	else{
	    printf("Error in command input\n");
	    exit(0);
	}
	n++;
    }
}

//This function is used to handle the file permissions
void printpermission(mode_t value){
    if(S_ISDIR(value)){
	printf("d");
    }
    else{
	printf("-");
    }
    printf((value & S_IRUSR) ? "r" : "-");
    printf((value & S_IWUSR) ? "w" : "-");
    printf((value & S_IXUSR) ? "x" : "-");
    printf((value & S_IRGRP) ? "r" : "-");
    printf((value & S_IWGRP) ? "w" : "-");
    printf((value & S_IXGRP) ? "x" : "-");
    printf((value & S_IROTH) ? "r" : "-");
    printf((value & S_IWOTH) ? "w" : "-");
    printf((value & S_IXOTH) ? "x" : "-");
    printf(" ");
}

//This function prints the date with the proper technique
void dateformat(char* date, time_t moddate){
    strftime(date,50,"%b %e %Y %H:%M", localtime(&moddate));
    printf("%s ",date);
    return;
}

//prints the ls command
void printstatement(bool iflag, bool lflag, bool Rflag, DIR* p){
    struct dirent* d;
    //while directory has files in it
    while(d=readdir(p)){
	if(strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0){
	    continue;
	}
	//if i option is enabled
	if(iflag == true){
	    printf("%lu ",d->d_ino);
	}

	//if l option is enabled
	if(lflag == true){
	    struct stat filestat;
	    stat(d->d_name, &filestat);

	    //prints the permissions
	    printpermission(filestat.st_mode);

	    //prints the link count
	    printf("%lu ",filestat.st_nlink);

	    //prints the username that owns the file
	    struct passwd* name;
    	    name = getpwuid(filestat.st_uid);
	    if(name != 0){
	        printf("%s ",name->pw_name);
	    }

	    //prints the group that the file belongs to
	    struct group* grp;
	    grp = getgrgid(filestat.st_gid);
	    if(grp != 0){
	        printf("%s ",grp->gr_name);
	    }

	    //prints the size in bytes
	    printf("%ld\t",filestat.st_size);

	    //prints the last date of modification
	    char date[50];
	    dateformat(date, filestat.st_mtime);
	}

	//print the name of the file
	printf("%s",d->d_name);
	if(lflag == true){
	    struct stat islink;
	    if (S_ISLNK(islink.st_mode) == 1){
		char buf[1024];
		char *path = realpath(d->d_name, buf);
		printf(" -> %s", buf);
	    }
	}
	printf("\n");
    }
    if (Rflag==false){
        closedir(p);
    }
    return;
}

//This function handles the R option
void recursive(bool iflag, bool lflag, bool Rflag, DIR* p, char* path){
    printf("%s:\n",path); 
    printstatement(iflag,lflag,Rflag,p);
    printf("\n");
    rewinddir(p);
    struct dirent* d;

    //while directory has files in it
    while(d=readdir(p)){
	struct stat dirstat;

	//If the file is a directory, open it if its not current directory or previous directory
	if(d->d_type == DT_DIR){
	    if(strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0){
		continue;
	    }
	    char next[1024];
	    strcpy(next,path);
	    strcat(next,d->d_name);
	    DIR* test;
	    test = opendir(next);
	    recursive(iflag,lflag,Rflag,test,next);
	}
    }
    closedir(p);
    return;
}



int main(int argc, char *argv[] ){
    //the flags are used to handle the options
    bool iflag = false;
    bool lflag = false;
    bool Rflag = false;

    //this value tests if their are arguments for the current directory(end of command line)
    bool dirflag = false;

    //this tests
    int count = 0;

    DIR* p;

    //if no arguments given
    if (argc == 1){
	p=opendir(".");
	if(p==NULL){
	    printf("Couldn't find current directory\n");
	    exit(0);
	}
	printstatement(iflag,lflag,Rflag,p);
    }

    else{
        int n=1;
	//while there are still arguments
        while(n<argc){
	    //If first character is dash, enter appropriate flags for the folder
	    if(argv[n][0] == '-'){
	        testinput(&iflag, &lflag, &Rflag, argv[n]);
		dirflag=true;
	    }
	    else{
		p=opendir(argv[n]);
		if(p==NULL){
		    printf("Couldn't find directory: %s\n",argv[n]);
		    exit(0);
		}
		
		//if R option is requested
		if (Rflag == true){
		    recursive(iflag,lflag,Rflag,p,argv[n]);
		}
		//Tests if there are multiple directories to print
		else{
		    if(n != argc-1 || count != 0){
			printf("%s:\n",argv[n]);
		    }
		    printstatement(iflag,lflag,Rflag,p);
		    if (n!= argc-1){
			printf("\n");
		    }
		}
		Rflag = false;
		iflag = false;
		lflag = false;
		dirflag = false;
		count++;
	    }
	    n++;
        }
	//if there is options hanging on no directory at the end of the command line
	if (dirflag == true){
            //sets appropriate options to true
            p=opendir(".");
            if(p==NULL){
	        printf("Couldn't find current directory\n");
	        exit(0);
            }

            //if R option is requested
            if(Rflag == true){
	        recursive(iflag,lflag,Rflag,p,".");
            }
            else{
		if(count!=0){
		    printf(".:\n");
		}
	        printstatement(iflag,lflag,Rflag,p);
            }
	}
    }
    return 0;
}
