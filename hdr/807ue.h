#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <bitset>
#include <sstream>
#include <osx_elf.h>
#define X86_64 2
#define X86 1

using namespace std;

#define DEBUG 1
#define CORE 1
#define REG_PREFIX 0xE0
#define VIRT_ADDR 0x1000
#define VADDR 0x400000
#define MAX_RAM 4880000000 //4.88 GB
#define STACK_SIZE 0x100000000+1024 //default linux stack size 
# define R15	0
# define R14	1
# define R13	2
# define R12	3
# define RBP	4
# define RBX	5
# define R11	6
# define R10	7
# define R9	8
# define R8	9
# define RAX	10
# define RCX	11
# define RDX	12
# define RSI	13
# define RDI	14
# define ORIG_RAX 15
# define RIP	16
# define CS	17
# define EFLAGS	18
# define RSP	19
# define SS	20
# define FS_BASE 21
# define GS_BASE 22
# define DS	23
# define ES	24
# define FS	25
# define GS	26
/*

    rsp - e
    rax - c



*/
# define dumpname(var, str)  sprintf(str, "%s", #var) 
char push_regs[15][4]={"rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi"};
unsigned char reg_names[12][4]={"rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi","es","ss","ds","rip"};
vector<pair<string, unsigned long long int> > registers;
//vector<pair<string, unsigned long long int> > movregs={{"rax",0xc5}{"rbp",0xec},{"rsp",0xe5},{"rcx",0xcd},{"rdx",0xd5},{"rbx",0xdd},{"rdi",0xfd},{"rsi",0xf5}};
vector<pair<string, unsigned long long int> > movregs={{"rax",0},{"rcx",1},{"rdx",2},{"rbx",3},{"rsp",4},{"rbp",5},{"rsi",6},{"rdi",7}};
//"rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi",rip,ss,ds,fs,gs,es
int bin_to_dec(string a){

    int res=0;
    int base =1 ;
    for(int i = a.length()-1;i>=0;i--){

        if(a[i] == '1'){
           
            res+=base;
        }
        base=base*2;
    
    }
    return res;


}

vector<pair<string,unsigned int> > syscalls={{"write",0x1}, {"read",0x0},{"open", 0x2},{"close",0x3},{"exit",60}};

class X86CPU{
    public:
        vector<string> instructions;
        vector<unsigned int> encoded;
        vector<unsigned int> stock_firm;
        unsigned char*stack;
        bool EXIT_FLAG=false;
        bool dbg;
        bool dmp_rg;

    
        unsigned int*  read_elf64(char *m){
            Elf64_Ehdr*ehdr = (Elf64_Ehdr*)m;
            Elf64_Shdr *shdr = (Elf64_Shdr*)(m+ehdr->e_shoff);
            Elf64_Shdr *shr = &shdr[ehdr->e_shstrndx];
            Elf64_Phdr *phdr = (Elf64_Phdr*)(m+ehdr->e_phoff);
            const char*shstr =(const char*)(m+shr->sh_offset);
            unsigned int addr =0 ;
            unsigned int offset=0;
            unsigned int size =0; 
            for(int i =0;i<ehdr->e_shnum;i++){
                string n = shstr+shdr[i].sh_name;
                if(n == ".text"){
                    addr = shdr[i].sh_addr;
                   size=shdr[i].sh_size;
                    offset = phdr[0].p_vaddr;
                    break;
                }

            }
            unsigned int r[2];
            
            addr=addr-offset;
            r[0]=addr;
            r[1]=size;
            return r;
        }
          unsigned int*  read_elf32(char *m){
            
            Elf32_Ehdr*ehdr = (Elf32_Ehdr*)m;
            Elf32_Shdr *shdr = (Elf32_Shdr*)(m+ehdr->e_shoff);
            Elf32_Shdr *shr = &shdr[ehdr->e_shstrndx];
            Elf32_Phdr *phdr = (Elf32_Phdr*)(m+ehdr->e_phoff);
            const char*shstr =(const char*)(m+shr->sh_offset);
            unsigned int addr =0 ;
            unsigned int offset=0;
            unsigned int size =0; 

            for(int i =0;i<ehdr->e_shnum;i++){
                string n = shstr+shdr[i].sh_name;
                if(n == ".text"){
                    addr = shdr[i].sh_addr;
                    size=shdr[i].sh_size;
                    offset = phdr[0].p_vaddr;
                    break;
                }

            }
            unsigned int r[2];
            
            addr=addr-offset;
            r[0]=addr;
            r[1]=size;
            return r;
            

        }
        void load_file(char *f){
            
            int fd= open(f,O_RDONLY);
            struct stat size;
            stat(f,&size);
            size_t dd= size.st_size;
            char* fmap=(char*)mmap(NULL,dd,PROT_READ,MAP_PRIVATE,fd,0);
            unsigned char *later=(unsigned char *)malloc(dd);
            memcpy(later,fmap,dd);
            Elf32_Ehdr *hdr =(Elf32_Ehdr*)fmap;
            int ver = hdr->e_ident[4];
            unsigned int offset = 0;
            int sizes=0;
            int start =0;
            if(ver == X86_64){
                unsigned int* ar= read_elf64(fmap);   
                offset=ar[0];
                sizes=ar[1];

                start=offset-sizeof(Elf64_Ehdr);

            }else{
                unsigned int* ar= read_elf32(fmap);   
                offset=ar[0];
                sizes=ar[1];
                start=offset-sizeof(Elf32_Ehdr);
            }
            if(dbg){
                printf("\n start:%d end:%d\n",start,offset+sizes);      
            }
            for(int i = 0;i<dd;i++){
                stock_firm.push_back(later[i]);
            }
            for(int i =offset-10;i<(offset+sizes);i++){
                encoded.push_back(later[i]);
            }  
        }  
        void init_stack(){
            stack = (unsigned char*)malloc(STACK_SIZE);
            if(stack == NULL){
                printf("\n*** unable to allocate default stack size.\n");
                exit(1);
            }
            memset(stack,0,STACK_SIZE);

        }
        void init_regs(){
            for(int i =0;i<12;i++){
                string t;
                for(int j =0;j<4;j++){
                    t+=reg_names[i][j];
                }
                registers.push_back(make_pair(t,0));
            }
           

        }
        void dump_regs(){
            int i = 0; 
            for(auto const& th:registers){
                if(i%4==0){
                    printf("\n");
                }
                printf("  %s:%ld  ",th.first.c_str(),th.second);
                i++;
            
            }
            printf("\n");
        }
        void execute_instructions(){
            init_regs();
            init_stack();
            registers[4].second = STACK_SIZE;
            registers[5].second=registers[4].second;
            unsigned long long int *rsp = &registers[4].second;
            unsigned long long int *rip = &registers[11].second;
            unsigned long long int *rbp = &registers[5].second;
            for(int i =0;i<encoded.size();i++){
                if(EXIT_FLAG){
                    exit(1);
                }
                if(dbg){
                    if(dmp_rg){
                      dump_regs();
                    }
                }
                string opcode;
                string oprand1;
                string oprand2;

                unsigned int byte = encoded[i];
 
                //printf("\nbyte:%lx\n",byte);
                if(byte >= 0x50 && byte<=0x57){

                    opcode = "push";    
                    byte =byte-0x50;
                    string reg_n = push_regs[byte];
                    oprand1=reg_n;
                    unsigned int found_val ;
                    for(auto const& th:registers){
                        if(th.first.find(reg_n) != string::npos){
                            found_val=th.second;
                            break;
                        }
                    }
                 
                    oprand1 = push_regs[byte]; 
                    registers[4].second-=4;

                    stack[registers[4].second]=found_val;
                    (*rip)++;
                } 
                if(byte >= 0x58 && byte<=0x5f){
                    opcode = "pop";    
                    byte =byte-0x58;
                    string reg_n = push_regs[byte];
                    oprand1=reg_n;
            
                    registers[byte].second=stack[(*rsp)];
                    (*rsp)+=4;                    
                    (*rip)++;
                }   
                if(byte >=0xb8 && byte <=0xb8+7){

                    opcode = "movn";
                
                    byte =byte-0xb8;
                    unsigned int number;

                    unsigned char t[4];
                    t[0] = encoded[i+4];
                    t[1] = encoded[i+3];
                    t[2] = encoded[i+2];
                    t[3] = encoded[i+1];
                    string m ;
                    for(int j = 0;j<4;j++){
                        string d;
                        char b[6];
                        sprintf(b,"%x",t[j]);
                        d = b;
                        if(d.length() == 1){
                            d="0"+d;
                        }
                        m+=d;   
                    }

                    stringstream ss;
                    ss<<hex<<m;
                    
                    unsigned int h;
                    ss>>h;

                 
                    registers[byte].second = h;
                    oprand1=registers[byte].first;
                    oprand2=to_string(h);
                    (*rip)++;
                    //  exit(1);
                }
               // printf("\n%x\n",registers[0].second);
              
                if(byte == 0xf && encoded[i+1]==0x5){
             
                    string name;
                    for(auto const&th:syscalls){
                        if(th.second == registers[0].second){
                            name = th.first;
                            break;
                        }
                    }

                    //rdi - first arg (fd), rsi-second arg,edx- third arg.
                    unsigned long long * rdi = &registers[7].second;//rdi
                    unsigned long long* rsi = &registers[6].second;//rsi
                    unsigned long long* edx = &registers[2].second;//rdx
                    opcode="syscall";
                    if(name =="open"){

                        unsigned int addr = (*rdi)-VADDR;
                        int len=0;
                        int b= 0; 
                        while(1){
                            if(stock_firm[addr+b]=='\0'){
                                break;
                            }
                            else{
                                len++;
                            }
                            b++;
                        }
                        char* fname = (char*)malloc(len);
                        for(int j = 0;j<len;j++){
                            fname[j]=stock_firm[addr+j];
                        }

                        int fd = open(fname,(*rsi),(*edx));
                        registers[0].second = fd;
                        free(fname);

                        //exit(1);
                    }
                    if(name == "write"){
                     
                        int len = (*edx);
                        unsigned int addr = (*rsi)-VADDR;
                        char* buf_fd=(char*)malloc((*edx));

                        for(int j = 0;j<len;j++){
                            buf_fd[j]=stock_firm[addr+j];
                        }   
                    
                        write((*rdi),buf_fd,len);
                        free(buf_fd);

                    }
                    if(name == "read"){
                        int len = (*edx);
                        unsigned int addr = (*rsi)-VADDR;
                        char* buf_fd=(char*)malloc((*edx));
                        read((*rdi),buf_fd,len);
                        free(buf_fd);
                    }
                    if(name == "exit"){
 
                        EXIT_FLAG=true;
                    }
                    if(name=="close"){
                        close((*rdi));
                    }
              
                    (*rip)++;
                }
                
                if(byte == 0x48){
                    //long mode.
                    if(encoded[i+1] == 0x8b){
                        
                        unsigned int oprands = encoded[i+2];
                        string res = bitset<8>(oprands).to_string();
                        string o1;
                         for(int j =2;j<5;j++){
                            o1+= res[j];
                        }
                        int op1=0;
                        op1 = bin_to_dec(o1);
                        string addr;
                         for(int j = 7   ;j>=4;j--){
                            char str[5];
                            sprintf(str,"%x",encoded[i+j]);
                            string d = str;
                            if(d.length() == 1){
                                d= "0"+d;
                            }
                            addr+=d;
                        }
                        stringstream cc;
                        cc.clear();
                        cc<<hex<<addr;
                        unsigned int finad;
                        cc>>finad;
                        registers[op1].second = stack[finad];
                        (*rip)++;
                //        exit(1);
                    }
                    if(encoded[i+1] >=0xb8 && encoded[i+1] <=0xb8+7){

                        opcode = "movabs";
                        unsigned int n  =encoded[i+1]-0xb8;
                        unsigned int number;

                        unsigned char t[6];
                        t[0] = encoded[i+7];
                        t[1] = encoded[i+6];
                        t[2] = encoded[i+5];
                        t[3] = encoded[i+4];
                        t[4] = encoded[i+3];
                        t[5] = encoded[i+2];


                        string m ;
                        stringstream ss;
                       
                        for(int j = 0;j<6;j++){
                            string d;
                            char b[6];
                            sprintf(b,"%x",t[j]);
                            d = b;
                            if(d.length()== 1){
                                d="0"+d;
                            }
                            m+=d;
                        }

                        ss<<hex<<m;
                        unsigned int h;
                        ss>>h;
                        registers[n].second = h;
                      //  cout<<h<<endl;
                            
                       // cout<<"REG:"<<registers[1].first<<":"<<registers[1].second<<endl;
                        oprand1=registers[n].first;
                        oprand2=to_string(h);
                        (*rip)++;

                    }   
                    
                    if(encoded[i+1]==0x83){
                        opcode = "sub";
                        unsigned int oprands = encoded[i+2];
                        string res = bitset<8>(oprands).to_string();
                        string o1,o2;
                        for(int j =2;j<5;j++){
                            o1+= res[j];
                        }
                        for(int j =5;j<8;j++){
                            o2+= res[j];
                        }
                        int op1=0,op2=0;
                        op1 = bin_to_dec(o1);
                        unsigned int n= encoded[i+3];
                        registers[op1].second -=n;
                        oprand1=registers[op1].first;
                        oprand2=to_string(n);

                        //exit(1);
                        (*rip)++;
                    }
                    if(encoded[i+1]==29){
                        opcode = "mov";
                        unsigned int oprands = encoded[i+2];
                        string res = bitset<8>(oprands).to_string();
                        string o1,o2;
                        for(int j =2;j<5;j++){
                            o1+= res[j];
                        }
                        for(int j =5;j<8;j++){
                            o2+= res[j];
                        }
                        int op1=0,op2=0;
                        op2 = bin_to_dec(o1);
                        op1=bin_to_dec(o2);                     
                        for(auto const &th:movregs){
                            if(th.second == op1){
                                oprand1=th.first;
                            }
                            if(th.second == op2){
                                oprand2=th.first;
                            }
                        }
                        registers[op1].second -= registers[op2].second;
                        (*rip)++;
                    }
                    if(encoded[i+1]==0x89){
                        opcode = "mov";
                        unsigned int oprands = encoded[i+2];
                        string res = bitset<8>(oprands).to_string();
                        string o1,o2;
                        string flip ;
                        flip+= res[0];
                        flip+=res[1];

              
                        if(flip == "11"){
                            for(int j =2;j<5;j++){
                                o1+= res[j];
                            }
                            for(int j =5;j<8;j++){
                                o2+= res[j];
                            }
                            int op1=0,op2=0;
                            op2 = bin_to_dec(o1);
                            op1=bin_to_dec(o2);                     
                            for(auto const &th:movregs){
                                if(th.second == op1){
                                    oprand1=th.first;
                                }
                                if(th.second == op2){
                                    oprand2=th.first;
                                }
                            }
                            registers[op1].second = registers[op2].second;
                        }
                        if(flip == "00"){  

                            for(int j =2;j<5;j++){
                                o1+= res[j];
                            }
                            int op2 = bin_to_dec(o1);
                            string addr;
                            
                            if(encoded[i+3] == 0x25){
                                for(int j = 7   ;j>=4;j--){
                                    char str[5];
                                    sprintf(str,"%x",encoded[i+j]);
                                    string d = str;
                                    if(d.length() == 1){
                                        d= "0"+d;
                                    }
                                    addr+=d;
                                }
                            }
                            else{
                                for(int j = 7   ;j>=3;j--){
                                    char str[5];
                                    sprintf(str,"%x",encoded[i+j]);
                                    string d = str;
                                    if(d.length() == 1){
                                        d= "0"+d;
                                    }
                                    addr+=d;
                                }
                            }
                            stringstream cc;
                            cc.clear();
                            unsigned int retval;
                            cc<<hex<<addr;
                            cc>>retval;
                            stack[retval] = registers[op2].second;
                            oprand1=to_string(retval);
                            oprand2=registers[op2].first;
                        }
                        (*rip)++;
                        
                    }

                }
                if(dbg){
                    if(opcode.length() >1){
                        if(oprand2.length()>0){
                            cout<<opcode<<"\t"<<oprand1<<","<<oprand2<<endl;
                        }
                        else{
                            cout<<opcode<<"\t"<<oprand1<<endl;  
                        }
                    }
                    
                }
                
            }
            memset(stack,0,STACK_SIZE);
            free(stack);

           
        }
        

};