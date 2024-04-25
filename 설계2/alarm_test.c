#include "types.h"
#include "user.h"
#include "date.h" // include to get rtcdate structure

int main(int argc, char *argv[])
{
	int seconds; // argument that user input
	struct rtcdate r; // rtcdate variable for current date

	if(argc <= 1) // if there is no argument input
		exit(); // error checking

	seconds = atoi(argv[1]); // convert char variable to int

	alarm(seconds); // call alarm system call to count the seconds

	date(&r); // call date system call to get current date
	printf(1, "SSU_Alarm Start\n"); // print "SSU_Alarm Start"
	printf(1, "Current time : %d-%d-%d %d:%d:%d\n", r.year, r.month, r.day, r.hour, r.minute, r.second); // print current time before count the seconds

	while(1) // pause until the seconds passes
		;
	exit();
}
