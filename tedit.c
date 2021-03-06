#include "types.h"
#include "fs.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

enum file{PROG, WR_TARGET, RD_TARGET};
enum state{WAIT, WRITE};

#define BUF_SIZE 256

int main(int argc, char* argv[]){
    getarg();
    int fd_read, fd_write;
    char ch[4];
    int state = WAIT;
    char buf[BUF_SIZE];
    int new_file = 0;
    int line = 0;

    if(argc != 2 && argc != 3){
        printf(2, "usage: tedit [wr_file] [rd_file] | tedit [wr_file]\n");
        exit();
    }
    else if(argc == 2)
        new_file = 1;
    else if(argc == 3){
        new_file = 0;
        fd_read = open(argv[RD_TARGET], O_RDONLY);
        if(fd_read < 0){
            printf(2, "fail: open %s.\n", argv[RD_TARGET]);
            exit();
        }
    }

    fd_write = open(argv[WR_TARGET], O_WRONLY | O_CREATE);
    if(fd_write < 0){
        printf(2, "fail: create file.\n");
        exit();
    }
    while(1){
        switch(state){
            case WAIT:
                if(!new_file){
                    line++;
                    fgets(buf, BUF_SIZE, fd_read);
                    printf(0, "[line : %d] %s", line, buf);
                }
                while(1){
                    printf(0, "[i] : write | [q] : end | [n] : next line\tcmd :");
                    read(0, ch, 4);
                    if(ch[0] == 'i'){
                        state = WRITE;
                        break;
                    }
                    else if(ch[0] == 'n'){
                        state = WAIT;
                        if(new_file) printf(fd_write, "\n");
                        else printf(fd_write, "%s", buf);
                        break;
                    }
                    else if(ch[0] == 'q')
                        goto loop_end;
                    else
                        state = WAIT;
                }
                memset(buf, 0, BUF_SIZE);
                break;
            case WRITE:
                printf(0, "> ");
                read(0, buf, BUF_SIZE);
                //printf(1, "%s", buf);
                printf(fd_write, "%s", buf);
                memset(buf, 0, BUF_SIZE);
                printf(0, "\n");
                state = WAIT;
                break;
        }
    }
loop_end:
    if(!new_file) close(fd_read);
    close(fd_write);
    exit();
}

