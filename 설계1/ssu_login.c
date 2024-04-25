#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char userID[16][32];
char pwdID[16][32];
int stop_flag = 0;

void parse_word(int fd, char *dest){ // read words from given fd(input or files)
		for(int j = 0; ; j++){
				if(read(fd, &dest[j], 1) <= 0){
					stop_flag = 1;
					break;
				}
				if(dest[j] == ' ' || dest[j] == '\n'){
					dest[j] = '\0'; // checking the end of words
					break;
				}
		}
}

void get_user_list(){ // get user list from the file given
		int fd;

		fd = open("list.txt", O_RDONLY);

		if(fd < 0) {
			printf(1, "list.txt open error\n");
			exit();
		}

		for(int i = 0; i < 10; i++){ // list.txt에서 Username, Password 정보 userID, pwdID 저장
				if(stop_flag) break;
				parse_word(fd, userID[i]);
				parse_word(fd, pwdID[i]);
		}
}

int check_idpw(){ // checking id and pwd
		char id[32] = {0,}, pwd[32] = {0,};
		int check_id = 0, check_pwd = 0;

		printf(1, "Username: ");
		parse_word(0, id); // get username
		printf(1, "Password: ");
		parse_word(0, pwd); // get pwd

		for(int i = 0; i < 10; i++) {
				if(!strcmp(id, userID[i])) check_id = 1; // if id is correct
				if(!strcmp(pwd, pwdID[i])) check_pwd = 1; // if pwd is correct
				if(check_id && check_pwd) return 1;
		}
		return 0; // else
}

int main(int argc, char *argv[]){
		int pid, wpid;

		get_user_list(); // call the function to get user id and pwd saved in file

		while(1){
			if(check_idpw()) // id checking
				for(;;){ // copy from init.c
					printf(1, "init: starting sh\n");
					pid = fork();
					if(pid < 0){
						printf(1, "init: fork failed\n");
						exit();
					}
					if(pid == 0){
						exec("sh", argv);
						printf(1, "init: exec sh failed\n");
						exit();
					}
					while((wpid = wait()) >= 0 && wpid != pid)
						printf(1, "zombie!\n");
				}
		}
}		
