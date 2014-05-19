#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#define size 2
int main(){
   char inputfile_1[20]="main.c" ,outputfile_1[20]="token.txt";
   char buf[size];
   int fd1 ,fd2 ,fd3;
   if((fd1=open(inputfile_1,O_RDONLY))==-1){
      printf("cannot open file.");
      exit(1);
   }
   fd3=open(outputfile_1,O_CREAT|O_WRONLY,0644);
   while((fd2=read(fd1,buf,size-1))>0){//fd2 bytes
     buf[1]='\0'; 
     printf("lookahed is %s\n",buf); 
      
      write(fd3,buf,fd2);
   }
   close(fd1);
   close(fd3);
}
