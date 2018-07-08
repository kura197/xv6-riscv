
#include "types.h"
#include "fs.h"
//#include "header/riscv.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

enum file{PROG, SRC, DST};

struct dictinary{
    char name[32];
    int addr;
};
static int dict_num;
static struct dictinary dict[64];

void elf_init(int fd_dst, int addr);
int parse(int fd_dst, char str[], int addr, int exec);
int get_reg(char reg_name[]);
int gen_binary_R(int opcode, int rd, int funct3, int rs1, int rs2, int funct7);
int gen_binary_I(int opcode, int rd, int funct3, int rs1, int imm);
int gen_binary_S(int opcode, int funct3, int rs1, int rs2, int imm);
int gen_binary_B(int opcode, int funct3, int rs1, int rs2, int imm);
int gen_binary_U(int opcode, int rd, int imm);
int gen_binary_J(int opcode, int rd, int imm);
void str2bin(int fd_dst);
int count_str();
void set_dict(char* str, int num, int addr);
int search_dict(char* str);
char *mystrtok(char *str, char *str1, char *str2);
int istoken(char ch);

int main(int argc, char* argv[]){
    getarg();
    int fd_src, fd_dst;
    char line[128];
    char str[128];
    char left[128];
    //char *p;
    //int addr = 0;

    if(argc < 3){
        printf(2,"need src and dst names.\n");
        exit();
    }

    if((fd_src = open(argv[SRC], 0)) < 0){
        printf(2, "\"%s\" doesn't exist.\n", argv[SRC]);
        exit();
    }
    fd_dst = open(argv[DST], O_CREATE | O_RDWR);

    while(fgets(line, 128, fd_src) > 0){
        if(line[0] == '\n')
            continue;
        /*
        mystrtok(line, str, left);
        printf(2, "str = %s\n", str);
        while(1){
            if(str == 0)
                break;
            else{
                mystrtok(left, str, left);
                printf(2, "str = %s\n", str);
            }
        }
        printf(2, "\n");
        */
        /*
        if(str[0] == '.'){
            if(!strcmp(str, ".string"))
                addr += count_str();
        }
        else if(str[0] == '#')
            continue;
        else if((p = strchr(str, ':')) == 0)
            addr = parse(fd_dst, str, addr, 0);
        else
            set_dict(str, (p - str), addr);
        */
    }

/*
    if(fseek(fd_src, 0, SEEK_SET) != 0){
        printf(2, "src fdeek filed.\n");
        exit();
    }
*/
    elf_init(fd_dst, addr);

    addr = 0;
    while(fgets(line, 128, fd_src) > 0){
        if(line[0] == '\n')
            continue;
        str = strtok(line, ",\t \n");
        if(str[0] == '.'){
            if(!strcmp(str, ".string"))
                str2bin(fd_dst);
        }
        else if(str[0] == '#')
            continue;
        else if((p = strchr(str, ':')) == 0){
            addr = parse(fd_dst, str, addr, 1);
        }
    }

    int padding = 0;
    for(int i=0; i<50;i++)
        write(fd_dst, &padding, 4);

    close(fd_src);
    close(fd_dst);
    exit();
}

void set_dict(char* str, int num, int addr){
    dict[dict_num].addr = addr;
    strncpy(dict[dict_num].name, str, num);
    dict[dict_num].name[num] = '\0';  
    dict_num++;
}

int search_dict(char* str){
    int i;
    for(i = 0; i < dict_num; i++){
        if(!strcmp(str, dict[i].name))
            return dict[i].addr;
    }
    return -1;
}
/*
int parse(int fd_dst, char str[], int addr, int exec){
    char* operand;
    int rs1, rs2, rd, imm;
    int bin;
    if(!strcmp(str, "addi")){
        operand = strtok(NULL, ",\t ");
        rd = get_reg(operand);
        operand = strtok(NULL, ",\t ");
        rs1 = get_reg(operand);
        operand = strtok(NULL, ",\t \n");
        imm = atoi(operand);
        bin = gen_binary_I(0b0010011, rd, 0b000, rs1, imm);

    }
    //imcomplete
    else if(!strcmp(str, "li")){
        operand = strtok(NULL, ",\t ");
        rd = get_reg(operand);
        operand = strtok(NULL, ",\t \n");
        imm = atoi(operand);
        bin = gen_binary_I(0b0010011, rd, 0b000, 0, imm);

    }
    else if(!strcmp(str, "sw")){
        operand = strtok(NULL, ",\t ");
        rs2 = get_reg(operand);
        operand = strtok(NULL, ",\t()");
        imm = atoi(operand);
        operand = strtok(NULL, ",\t()");
        rs1 = get_reg(operand);
        bin = gen_binary_S(0b0100011, 0b010, rs2, rs1, imm);

    }
    else if(!strcmp(str, "lw")){
        operand = strtok(NULL, ",\t ");
        rd = get_reg(operand);
        operand = strtok(NULL, ",\t()");
        imm = atoi(operand);
        operand = strtok(NULL, ",\t()");
        rs1 = get_reg(operand);
        bin = gen_binary_I(0b0000011, rd, 0b010, rs1, imm);

    }
    else if(!strcmp(str, "nop")){
        rd = 0;
        rs1 = 0;
        imm = 0;
        bin = gen_binary_I(0b0010011, rd, 0b000, rs1, imm);

    }
    else if(!strcmp(str, "mv")){
        operand = strtok(NULL, ",\t ");
        rd = get_reg(operand);
        operand = strtok(NULL, ",\t \n");
        rs1 = get_reg(operand);
        imm = 0;
        bin = gen_binary_I(0b0010011, rd, 0b000, rs1, imm);

    }
    else if(!strcmp(str, "sub")){
        operand = strtok(NULL, ",\t ");
        rd = get_reg(operand);
        operand = strtok(NULL, ",\t ");
        rs1 = get_reg(operand);
        operand = strtok(NULL, ",\t \n");
        rs2 = get_reg(operand);
        bin = gen_binary_R(0b0110011, rd, 0b000, rs1, rs2, 0b0100000);

    }
    else if(!strcmp(str, "jal")){
        operand = strtok(NULL, ",\t \n");
        char* operand2 = strtok(NULL, ",\t \n");
        if(operand2 == NULL){
            rd = 1;
            //imm = atoi(operand);
            imm = search_dict(operand);
        }else{
            rd = get_reg(operand);
            //imm = atoi(operand2);
            imm = search_dict(operand2);
        }
        bin = gen_binary_J(0b1101111, rd, imm);

    }
    else if(!strcmp(str, "j")){
        operand = strtok(NULL, ",\t \n");
        //imm = atoi(operand);
        imm = search_dict(operand);
        bin = gen_binary_J(0b1101111, 0, imm);

    }
    else if(!strcmp(str, "jalr")){
        operand = strtok(NULL, ",\t ");
        char *operand2 = strtok(NULL, ",\t ");
        if(operand2 == NULL){
            rd = 1;
            rs1 = get_reg(operand);
            imm = 0;
        }else{
            rd = get_reg(operand);
            rs1 = get_reg(operand2);
            operand = strtok(NULL, ",\t \n");
            imm = atoi(operand);
        }
        bin = gen_binary_I(0b1100111, rd, 0b000, rs1, imm);

    }
    else if(!strcmp(str, "jr")){
        operand = strtok(NULL, ",\t \n");
        rs1 = get_reg(operand);
        bin = gen_binary_I(0b1100111, 0, 0b000, rs1, 0);

    }
    else if(!strcmp(str, "ret")){
        bin = gen_binary_I(0b1100111, 0, 0b000, 1, 0);

    }
    else if(!strcmp(str, "auipc")){
        operand = strtok(NULL, ",\t ");
        rd = get_reg(operand);
        operand = strtok(NULL, ",\t \n");
        imm = atoi(operand);
        bin = gen_binary_U(0b0010111, rd, imm);

    }
    else if(!strcmp(str, "call")){
        operand = strtok(NULL, ",\t \n");
        imm = search_dict(operand);
        bin = gen_binary_U(0b0010111, 6, (imm >> 12));
        if(exec) write(fd_dst, &bin, 4);
        addr += 4;
        bin = gen_binary_I(0b1100111, 1, 0b000, 6, (imm & 0xfff));
    }
    else if(!strcmp(str, "ecall")){
        bin = 0x73;
    }
    else if(!strcmp(str, "la")){
        operand = strtok(NULL, ",\t ");
        rd = get_reg(operand);
        operand = strtok(NULL, ",\t \n");
        imm = search_dict(operand) - addr;
        bin = gen_binary_U(0b0010111, rd, (imm & ~0xfff));
        if(exec) write(fd_dst, &bin, 4);
        addr += 4;
        bin = gen_binary_I(0b0010011, rd, 0b000, rd, (imm & 0xfff));

    }
    else{
        return -1;
    }
    if(exec) write(fd_dst, &bin, 4);
    addr += 4;
    return addr;
}
*/
int gen_binary_R(int opcode, int rd, int funct3, int rs1, int rs2, int funct7){
    int bin;
    bin = (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
    return bin;
}

int gen_binary_I(int opcode, int rd, int funct3, int rs1, int imm){
    int bin;
    bin = (imm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
    return bin;
}

int gen_binary_S(int opcode, int funct3, int rs2, int rs1, int imm){
    int bin;
    bin = ((imm >> 5) << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | ((imm & 0x1f) << 7) | opcode;
    return bin;
}

int gen_binary_B(int opcode, int funct3, int rs1, int rs2, int imm){
    int bin;
    bin = ((imm >> 12) << 31) | (((imm & 0x7e) >> 5) << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | ((imm & 0x1e) << 8) | (((imm & 0x800) >> 11) << 7) | opcode;
    return bin;
}

int gen_binary_U(int opcode, int rd, int imm){
    int bin;
    bin = (imm & ~0xfff) | (rd << 7) | opcode;
    return bin;
}

int gen_binary_J(int opcode, int rd, int imm){
    int bin;
    bin = ((imm >> 20) << 31) | (((imm & 0x7fe) >> 1) << 21) | (((imm & 0x800) >> 11) << 20) | (imm & 0xff000) | (rd << 7) | opcode;
    return bin;
}
/*
void str2bin(int fd_dst){
    char *string = strtok(NULL, "\"");
    while(*string != '\0'){
        if(*string != '\\')
            write(fd_dst, string, 1);
        else if(*(string+1) == 'n'){
            char newline = '\n';
            write(fd_dst, &newline, 1);
            string++;
        }
        string++;
    }
}

int count_str(){
    int addr = 0;
    char *string = strtok(NULL, "\"");
    while(*string != '\0'){
        if(*string != '\\')
            addr += 1;
        else if(*(string+1) == 'n'){
            addr += 1;
            string++;
        }
        string++;
    }
    return addr;
}
*/

int get_reg(char reg_name[]){
    if(!strcmp(reg_name, "x0") || !strcmp(reg_name, "zero"))
        return 0;
    else if(!strcmp(reg_name, "x1") || !strcmp(reg_name, "ra"))
        return 1;
    else if(!strcmp(reg_name, "x2") || !strcmp(reg_name, "sp"))
        return 2;
    else if(!strcmp(reg_name, "x3") || !strcmp(reg_name, "gp"))
        return 3;
    else if(!strcmp(reg_name, "x4") || !strcmp(reg_name, "tp"))
        return 4;
    else if(!strcmp(reg_name, "x5") || !strcmp(reg_name, "t0"))
        return 5;
    else if(!strcmp(reg_name, "x6") || !strcmp(reg_name, "t1"))
        return 6;
    else if(!strcmp(reg_name, "x7") || !strcmp(reg_name, "t2"))
        return 7;
    else if(!strcmp(reg_name, "x8") || !strcmp(reg_name, "s0") || !strcmp(reg_name, "fp"))
        return 8;
    else if(!strcmp(reg_name, "x9") || !strcmp(reg_name, "s1"))
        return 9;
    else if(!strcmp(reg_name, "x10") || !strcmp(reg_name, "a0"))
        return 10;
    else if(!strcmp(reg_name, "x11") || !strcmp(reg_name, "a1"))
        return 11;
    else if(!strcmp(reg_name, "x12") || !strcmp(reg_name, "a2"))
        return 12;
    else if(!strcmp(reg_name, "x13") || !strcmp(reg_name, "a3"))
        return 13;
    else if(!strcmp(reg_name, "x14") || !strcmp(reg_name, "a4"))
        return 14;
    else if(!strcmp(reg_name, "x15") || !strcmp(reg_name, "a5"))
        return 15;
    else if(!strcmp(reg_name, "x16") || !strcmp(reg_name, "a6"))
        return 16;
    else if(!strcmp(reg_name, "x17") || !strcmp(reg_name, "a7"))
        return 17;
    else if(!strcmp(reg_name, "x18") || !strcmp(reg_name, "s2"))
        return 18;
    else if(!strcmp(reg_name, "x19") || !strcmp(reg_name, "s3"))
        return 19;
    else if(!strcmp(reg_name, "x20") || !strcmp(reg_name, "s4"))
        return 20;
    else if(!strcmp(reg_name, "x21") || !strcmp(reg_name, "s5"))
        return 21;
    else if(!strcmp(reg_name, "x22") || !strcmp(reg_name, "s6"))
        return 22;
    else if(!strcmp(reg_name, "x23") || !strcmp(reg_name, "s7"))
        return 23;
    else if(!strcmp(reg_name, "x24") || !strcmp(reg_name, "s8"))
        return 24;
    else if(!strcmp(reg_name, "x25") || !strcmp(reg_name, "s9"))
        return 25;
    else if(!strcmp(reg_name, "x26") || !strcmp(reg_name, "s10"))
        return 26;
    else if(!strcmp(reg_name, "x27") || !strcmp(reg_name, "s11"))
        return 27;
    else if(!strcmp(reg_name, "x28") || !strcmp(reg_name, "t3"))
        return 28;
    else if(!strcmp(reg_name, "x29") || !strcmp(reg_name, "t4"))
        return 29;
    else if(!strcmp(reg_name, "x30") || !strcmp(reg_name, "t5"))
        return 30;
    else if(!strcmp(reg_name, "x31") || !strcmp(reg_name, "t6"))
        return 31;
    else{
        printf(2, "register error.\n");
        return -1;
    }
}

void elf_init(int fd_dst, int addr){
    int i;
    int magic = 0x464C457FU;  // "\x7FELF" in little endian
    char class = 1;
    char data = 1;
    char elf_version = 1;
    char osabi = 0;
    char abiversion = 0;
    char padding = 0;

    short etype = 2;
    short machine = 0xf3;
    int version = 1; 
    int entry = 0;
    int phoff = 0x34;
    int shoff = 0x0;
    int flags = 0x04;
    short ehsize = 0x34;
    short phentsize = 0x20;
    short phnum = 0x01;
    short shentsize = 0x28;
    short shnum = 0x0;
    short shstrndx = 0x0f;

    int p_type = 0x01;
    int p_offdet = 0x54;
    int p_vaddr = 0x00;
    int p_paddr = 0x00;
    int p_filesz = addr + p_offdet;
    int p_memsz = addr + p_offdet;
    int p_flags = 0x07;
    int p_align = 0x04;

    write(fd_dst, &magic,  4);
    write(fd_dst, &class,  1);
    write(fd_dst, &data,  1);
    write(fd_dst, &elf_version,  1);
    write(fd_dst, &osabi,  1);
    write(fd_dst, &abiversion,  1);
    for(i = 0; i < 7; i++)
        write(fd_dst, &padding,  1);

    write(fd_dst, &etype,  2);
    write(fd_dst, &machine,  2);
    write(fd_dst, &version,  4);
    write(fd_dst, &entry,  4);
    write(fd_dst, &phoff,  4);
    write(fd_dst, &shoff,  4);
    write(fd_dst, &flags,  4);
    write(fd_dst, &ehsize,  2);
    write(fd_dst, &phentsize,  2);
    write(fd_dst, &phnum,  2);
    write(fd_dst, &shentsize,  2);
    write(fd_dst, &shnum,  2);
    write(fd_dst, &shstrndx,  2);

    write(fd_dst, &p_type,  4);
    write(fd_dst, &p_offdet,  4);
    write(fd_dst, &p_vaddr,  4);
    write(fd_dst, &p_paddr,  4);
    write(fd_dst, &p_filesz,  4);
    write(fd_dst, &p_memsz,  4);
    write(fd_dst, &p_flags,  4);
    write(fd_dst, &p_align,  4);


}

char *mystrtok(char *str, char *str1, char *str2){
    char *idx = str;
    while(1){
        if(istoken(*idx))   idx++;
        else    break;
    }

    //idx points top of a word
    char *idx2 = idx + 2;
    str2 = 0;
    str1 = 0;
    while(*idx2 != '\0'){
        if(istoken(*idx2)){
            strncpy(str1, idx, idx2-idx); 
            str1[idx2-idx] = '\0';
            strcpy(str2, idx2);
            break;
        }
        idx2++;
    }

    return str1;
}

int istoken(char ch){
    if(ch == ' ' || ch == ',' || ch == '\t' || ch == '\n')
        return 1;
    else
        return 0;
}
