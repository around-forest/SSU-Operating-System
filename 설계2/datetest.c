#include "types.h"
#include "stat.h"
#include "user.h"
#include "date.h" // header for rtcdate structure

int
main(int argc, char *argv[])
{
	struct rtcdate r; // rtcdate variable for current date and time
	if(date(&r)){ // get current date and time by calling new system call : date()
		printf(1, "date: system call error\n"); // error checking
		exit();
	}
 // print the current date and time info got from date system call
	printf(1, "Current time : %d-%d-%d %d:%d:%d\n", r.year, r.month, r.day, r.hour, r.minute, r.second);

	exit();
}
