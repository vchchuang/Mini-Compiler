#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#define chstr_size 2
#define buf_size 512
char inputfile_2[20]="grammar.txt" ,outputfile_2[20]="set.txt";
char chstr[chstr_size] ,buf[buf_size];
int fd1 ,fd2 ,fd3;
struct RHS{
   char element[20];
};
struct LHS{
   char name[20];
   struct RHS body[5];
}NonT;
void checkpoint(char* s){
   printf("lookahead is %s\n",s);
}

void init(){
   //open inputfile
   if((fd1=open(inputfile_2,O_RDONLY))==-1){
      printf("cannot open file.");
      exit(1);
   }
   //open outputfile
   fd3=open(outputfile_2,O_CREAT|O_WRONLY,0644);
   //read data
   while((fd2=read(fd1 ,chstr,chstr_size-1))>0){//fd2 bytes
      chstr[1]='\0';
      checkpoint(chstr);
/*      if(chstr!=" "){

      }*/
   }
}
int main(){
   init();
   
   close(fd1);
   close(fd3);
}
