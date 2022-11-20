#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <807ue.h>

using namespace std;


int main(int argc,char *argv[]){


    if(argc < 2){
        printf("\nprovide the filename of the executable");
    }
    X86CPU *cpu= new X86CPU();
    cpu->load_file(argv[1]);
    //cpu->dbg= true;
   // cpu->dmp_rg=true;
    cpu->execute_instructions();

}
