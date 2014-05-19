#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#define chstr_size 2
#define buf_size 512
char inputfile_2[20]="grammar.txt" ,outputfile_2[20]="set.txt";
char chstr[chstr_size] ,buf[buf_size];
int fd1 ,fd2 ,fd3;
struct RHS{
   char element[10][20];
};
struct LHS{
   char name[20];
   struct RHS body[15];
}NonT[25];
void checkpoint(int a ,char* s){
   printf("lookahead is %s in state %d \n",s,a);
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
   int ch_count = 0 ,isLHS = 1 ,elem_count = 0 , p_count = 0,nont_count = 0;
   int pass_b = 1;//pass_b is the position of the blank
   int state = 0 ,s_count = 0;
   while((fd2=read(fd1 ,chstr,chstr_size-1))>0){//fd2 bytes
      chstr[1]='\0';
      switch(state){
      case 0://LHS
         checkpoint(state,chstr);
            if(strcmp(chstr," ")!=0&&strcmp(chstr,"\n")!=0){
               NonT[nont_count].name[ch_count]=chstr[0];
               ch_count = ch_count + 1;
            }else if(strcmp(chstr,"\n")==0){
               NonT[nont_count].name[ch_count]='\0';
               ch_count = 0;
               state = 1;
            }
            break;
      case 1://RHS
            checkpoint(state,chstr);
            if(strcmp(chstr," ")==0&&s_count<4){
               s_count = s_count + 1;
               state=1;
            }
            else if(s_count==4){//ch
               s_count = 0;
               NonT[nont_count].body[p_count].element[elem_count][ch_count]
                 =chstr[0];
               ch_count =ch_count + 1;
               state = 2;
            }
            break;
      case 2:
            checkpoint(state,chstr);
            if(strcmp(chstr," ")!=0&&strcmp(chstr,"\n")!=0){
                NonT[nont_count].body[p_count].element[elem_count][ch_count]
                 =chstr[0];
                ch_count = ch_count + 1;
                state = 2;
            }else if(strcmp(chstr," ")==0) {
                NonT[nont_count].body[p_count].element[elem_count][ch_count]
                 ='\0';
                ch_count = 0;
                elem_count = elem_count + 1;
                state = 2;
            }else if(strcmp(chstr,"\n")==0){//one prodution finished
                NonT[nont_count].body[p_count].element[elem_count][ch_count]
                ='\0';
                ch_count = 0;
                elem_count = 0;
                state = 4;
            }
            break;
      case 4://4 -> 0 next Non-t //4 -> 2 next prod in the same Non-t
            checkpoint(state,chstr);
             if(strcmp(chstr," ")==0&&s_count<4){
               s_count = s_count + 1;
               state = 4;
            }
             else if(s_count==4){//next production
               s_count = 0;
               p_count = p_count + 1;
               NonT[nont_count].body[p_count].element[elem_count][ch_count]
                =chstr[0];
               ch_count = ch_count + 1;
               state = 2;
            }else if(strcmp(chstr,"\n")!=0){//next Non-terminal
               nont_count = nont_count + 1;
               NonT[nont_count].name[ch_count] = chstr[0];
               ch_count = ch_count + 1;
               state = 0;
            }
             break;
      }
   }
   int loop_count = 0;
   
   for(;loop_count<25;loop_count++){
      printf("%s\n",NonT[loop_count].name);
   }
}
int main(){
   init();
   
   close(fd1);
   close(fd3);
}
