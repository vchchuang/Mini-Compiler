#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#define chstr_size 2
#define buf_size 512
char inputfile_1[20]="main.c" ,outputfile_1[20]="token.txt";
char inputfile_2[20]="grammar.txt" ,outputfile_2[20]="set.txt";
char chstr[chstr_size] ,buf[buf_size];
int fd1 ,fd2 ,fd3;//IO grammar information
int fd4 ,fd5 ,fd6;//IO source code
struct RHS{
   char element[10][20];
   int e_max;
};
struct LHS{
   char name[20];
   struct RHS body[15];
   int p_max;
}NonT[28];
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
   //init info
   for(int i=0;i<20;i++){
      NonT[i].body[p_count].e_max = 0;
      NonT[i].p_max = 0;
   }
   while((fd2=read(fd1 ,chstr,chstr_size-1))>0){//fd2 bytes
      chstr[1]='\0';
      switch(state){
      case 0://LHS
//         checkpoint(state,chstr);
            //start analysis a production
            if(strcmp(chstr," ")!=0&&strcmp(chstr,"\n")!=0){
               NonT[nont_count].name[ch_count]=chstr[0];
               ch_count = ch_count + 1;
            }else if(strcmp(chstr,"\n")==0){
               NonT[nont_count].name[ch_count]='\0';
              printf("%s\n\t",NonT[nont_count].name); //printf LHS
               ch_count = 0;
               state = 1;
            }
            break;
      case 1://RHS
//            checkpoint(state,chstr);
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
  //          checkpoint(state,chstr);
            if(strcmp(chstr," ")!=0&&strcmp(chstr,"\n")!=0){
                NonT[nont_count].body[p_count].element[elem_count][ch_count]
                 =chstr[0];
                ch_count = ch_count + 1;
                state = 2;
            }else if(strcmp(chstr," ")==0) {//the part of body
                NonT[nont_count].body[p_count].element[elem_count][ch_count]
                 ='\0';
                ch_count = 0;
                printf("elem %d :: %s\t",elem_count,NonT[nont_count].body[p_count].element[elem_count]);//printf
                //memo counting data
                NonT[nont_count].body[p_count].e_max
                  = elem_count = elem_count + 1;
                state = 2;
            }else if(strcmp(chstr,"\n")==0){//one body finished
                NonT[nont_count].body[p_count].element[elem_count][ch_count]
                ='\0';
                //memo counting data
                NonT[nont_count].p_max 
                  = p_count + 1;
                printf("elem %d :: %s\t//// prod num > %d \n\n",elem_count,NonT[nont_count].body[p_count].element[elem_count],p_count);//printf
                ch_count = 0;
                elem_count = 0;
                state = 4;//if there is a char then keep going!!
            }
            break;
      case 4://4 -> 0 next Non-t //4 -> 2 next prod in the same Non-t
    //        checkpoint(state,chstr);
             if(strcmp(chstr," ")==0&&s_count<4){
               s_count = s_count + 1;
               state = 4;
            }
             else if(s_count==4){//next production
               s_count = 0;
               //memo counting data
               NonT[nont_count].p_max 
                  = p_count = p_count + 1;
               
               NonT[nont_count].body[p_count].element[elem_count][ch_count]
                =chstr[0];
               printf("\t");//printf next prod
               ch_count = ch_count + 1;
               state = 2;
            }else if(strcmp(chstr,"\n")!=0){//next Non-terminal
               p_count = 0;
               nont_count = nont_count + 1;
               NonT[nont_count].name[ch_count] = chstr[0];
               ch_count = ch_count + 1;
               state = 0;
            }
             break;
      }
   }
  //check data structure
   int loop_count = 0;
   for(;loop_count<=26;loop_count++){
      printf("%s\n",NonT[loop_count].name);
      for(int p=0;p<=NonT[loop_count].p_max;p++){
         printf("\t");
         for(int e=0;e<=NonT[loop_count].body[p].e_max;e++){
            printf("%s  ",NonT[loop_count].body[p].element[e]);
         }
         printf("\n");
      }
      printf("\n");
   }
}
int main(){
   //build parsing table
   init();
   //open inputfile
   if((fd4=open(inputfile_1,O_RDONLY))==-1){
      printf("cannot open file.");
      exit(1);
   }
   //open outputfile
   fd6=open(outputfile_1,O_CREAT|O_WRONLY,0644);
   
   //parsing
   while((fd5=read(fd4 ,chstr,chstr_size-1))>0){//fd6 bytes
      //check lexem and build token list ,symbol table
      
      //LL(1) parser
      
      
      
      //write(fd6,chstr,chstr_size-1);
   }

   close(fd1);
   close(fd3);
   close(fd4);
   close(fd6);
}
