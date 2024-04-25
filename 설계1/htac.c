#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char buf[1];
int endLine, totalLine = 0;

void
htac(int fd, char *argv)
{
  int n, cntLine;

	for(int i = 0; i < endLine; i++) {
		cntLine = 0;
		while(read(fd, buf, sizeof(buf)) > 0) { // moving offset to the next line
			if (buf[0] == '\n'){
				cntLine++;
				if(cntLine == (totalLine - i)) break; // bottom - up
			}
		}

		while((n = read(fd, buf, sizeof(buf))) > 0) { // print words
			if (buf[0] == '\n')	break; // until it reaches '\n' -> printing 1 line
			if (write(1, buf, n) != n) {
				printf(1, "cat: write error\n");
				exit();
			}
		}
		printf(1, "\n"); // line changing
		if(n < 0){
			printf(1, "cat: read error\n");
			exit();
		}
		close(fd); // close to move the offset to the beginning
		if((fd = open(argv, 0)) < 0){ // open again
			printf(1, "cat: cannot open %s\n", argv);
			exit();
		}
	}
}

int
main(int argc, char *argv[])
{
  int fd, i;

  if(argc <= 2){
    htac(0, 0);
    exit();
  }

	endLine = atoi(argv[1]); // get the number of lines that user wants

  for(i = 2; i < argc; i++){
    if((fd = open(argv[i], 0)) < 0){
      printf(1, "cat: cannot open %s\n", argv[i]);
      exit();
    }
		while(read(fd, buf, sizeof(buf)) > 0) {
			if (buf[0] == '\n') totalLine++; // getting total lines in the file
		}
		close(fd); // close to reset the offset

		if((fd = open(argv[i], 0)) < 0){ // open again
			printf(1, "cat: cannot open %s\n", argv[i]);
			exit();
		}
    htac(fd, argv[i]);
    close(fd);
  }
  exit();
}
