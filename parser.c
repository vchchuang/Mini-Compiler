#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#define chstr_size 2
#define buf_size 10
#define g_size 28
#define set_size 40
#define loc_size 20
#define nol 50
#define st_size 50
char inputfile_1[20]="main.c" ,outputfile_1[20]="token.txt";
char inputfile_2[20]="grammar.txt" ,outputfile_2[20]="set.txt";
char outputfile_3[20]="symbol.txt";
//For lexer
char chstr[chstr_size] ,buf[buf_size];
//For IO File
int fd1 ,fd2 ,fd3;//IO grammar information
int fd4 ,fd5 ,fd6;//IO source code
char *tok_dict[]={
   "int","char","return","if","else","while","break"//Keyword
   ,"=","!","+","-","*","/","==","!=","<",">","<=",">=","&&","||"//Operator
     ,"[","]","(",")","{","}",",",";"//Special Symbol
     ,"id"//Identifier
     ,"num"//Number
     ,"CHAR"//Char
     ,"//"//Comment
     ,"epsilon"//empty string
     ,"$"
};
char* tok_sym[9]={"Keyword","Operator","SpecialSymbol","Identifier","Num","Char","Comment","Epsilon","$"};

int state=0 ,lookahead=-1;
int line=0 ,it=0 ,sc=0 ,scope=0;
int st_count =0;
struct symbol_table{
   char symbol[10]; 
   char* token;
   char type[10];
   int scope;
   //int value;
}st[20];
struct lex_data{//for lexer
   int token;
   int scope;
   char name[buf_size];
};
struct line_lex{
   struct lex_data item[loc_size];
   int e_count;
   int line;
}tok_list[nol];

struct set{
   char* element[set_size];
   int set_max;
};
struct RHS{
   char element[10][20];
   int e_max;
   int isNull;
   struct set First;
   struct set Follow;
};
struct LHS{
   char name[20];
   struct RHS body[15];
   int p_max;
   int isNull;
   struct set First;
   struct set Follow;
}NonT[g_size];

//STACK
int stack[st_size];//if stack[stp-1] is '$' => parsing finished!
int stp=0;
//STACK OP
void push(int i){
   if(stp>=st_size){
      printf("stack overflow!! current is %d\n",i);
   }else{
      stack[stp++]=i;
   }
}
void rm(int hm){
   if(stp<=(st_size-st_size)){
      printf("stack is empty!! \n");
   }else{
      stp = stp-hm;
   }
}

//search value
int FindTokV(char* str){
   int tokv=0 ,c=0;
   while(strcmp(str,tok_dict[c])!=0&&strcmp(tok_dict[c],"$")!=0){
      c = c+1;
   }
   if(c>50){
      return -1;
   }else{
      tokv = c+40;
      return tokv;//tokv-40+1 => serial num of terminal
   }
}

int FindNonTV(char* str){
   int nontv=0 ,c=0;
   while(strcmp(str,NonT[c].name)!=0&&c<g_size){
      c = c+1;
   }
   if(c>=g_size){
      return -1;
   }else{
      nontv = c;
      return nontv;
   }
}

//for input
void checkpoint(int a ,char* s){
   printf("lookahead is %s in state %d \n",s,a);
}

int checkSymbolTable(char* str){

   for(int i=0;i<st_count;i++){
      if(strcmp(st[i].symbol,str)==0)return 0;
   }
   return 1;//not found in symbol table
}
int isT(char* str){
   for(int i=0;i<g_size;i++){
      if(strcmp(NonT[i].name,str)==0){
         return i;
      }
   }
   return 30;
}
void addSet(struct set* SET1,struct set SET2){
   int check=0;
   if(SET2.set_max==0){
      return 0;
   }else{

      for(int i=0;i<SET2.set_max;i++){
         //check
         check=0;
         for(int j=0;j<SET1->set_max;j++){
            if(strcmp(SET1->element[j],SET2.element[i])==0){
               check=1;
               break;
            }
         }
         //put into the set1
         if(check==0){
            SET1->element[SET1->set_max]=SET2.element[i];
            SET1->set_max = SET1->set_max + 1;
         }
         //check next
      }
   }
}

int Nullable(char* str){ 
   int index=0 ,nr=0;
   index = isT(str);
   if(strcmp(str,"epsilon")==0){
      return 1;//true
   }
   if(index==30){
      return 0;//false
   }
//    printf("%d %s -",index,str);
   //NonTer
   for(int or=0;or<NonT[index].p_max;or++){//for body1 | body2 | ... | bodyn
      for(int and=0;and<=NonT[index].body[or].e_max;and++){//for e1 ^ e2 ^ ... ^en
         if((nr=Nullable(NonT[index].body[or].element[and]))==0){
     //       printf("%s ",NonT[index].body[or].element[and]);
            NonT[index].body[or].isNull=0;
            break;//check next body
         }else if(and==NonT[index].body[or].e_max){
            return 1;//check finished
         }
      }
   }
   return 0;
}
struct set First(char* str){
   int index = 0;
   struct set SET;
   printf("%d",index);
   index = isT(str);
   SET.set_max = 0;
   if(index==30){
      SET.element[0]=str;
      SET.set_max = 1;
      return SET;
   }else if(!NonT[index].isNull){
      return NonT[index].First;
   }else{
      SET.element[0]="epsilon";
      SET.set_max = 1;
      return SET;
   }
   return SET;
}
void Follow(char* lhs){
   int index=0 ,index2=0;
   struct set SET;
   index=isT(lhs);
   for(int i=0;i<g_size;i++){
      for(int pd=0;pd<NonT[i].p_max;pd++){
         for(int e=0;e<=NonT[i].body[pd].e_max;e++){
            //S -> a T 
            if(strcmp(NonT[index].name,NonT[i].body[pd].element[e])==0){
               
               if(e==NonT[i].body[pd].e_max){ 
                 // printf("NonT is %s and  is in %s 's body-%d ,%dth position\n" ,NonT[index].name,NonT[i].name,pd,e);
                  addSet(&NonT[index].Follow ,NonT[i].Follow);
               }else if(e<NonT[i].body[pd].e_max){
                  index2=isT(NonT[i].body[pd].element[e+1]);

                  if(index2==30){//S -> T b
                    printf("NonT is %s and  is in %s 's body-%d ,%dth position ,e+1 is %s\n" ,NonT[index].name,NonT[i].name,pd,e,NonT[i].body[pd].element[e+1]);
                     SET.element[0]=NonT[i].body[pd].element[e+1];
                     SET.set_max = 1;
                     addSet(&NonT[index].Follow ,SET);
                  }else { //S -> T B
                  // printf("NonT is %s and  is in %s 's body-%d ,%dth position\n" ,NonT[index].name,NonT[i].name,pd,e); 
                     if(!NonT[index2].isNull) addSet(&NonT[index].Follow ,NonT[index2].First);
                     else if(e+1==NonT[i].body[pd].e_max){
                        addSet(&NonT[index].Follow ,NonT[index2].First);
                        addSet(&NonT[index].Follow ,NonT[i].Follow);
                     }
                  }
               }
            } 
         }
      }
   }
}


void lexer(char* ch,int c){
   
   //printf("%s",buf);
  // printf("%d",ch);
   switch(state){
     case 0:
         if(strcmp(ch,"+")==0||strcmp(ch,"-")==0||strcmp(ch,"*")==0||strcmp(ch,"/")==0||strcmp(ch,"=")==0||strcmp(ch,";")==0||strcmp(ch,",")==0||strcmp(ch,"(")==0||strcmp(ch,")")==0||strcmp(ch,"{")==0||strcmp(ch,"}")==0){
            buf[c]=ch[0];
            buf[c+1]='\0';
//           printf("%s",buf);
            state = 1;// operator and special symbol
         }else if(isalpha(ch[0])){
            buf[c]=ch[0];
            buf[c+1]='\0';
            state = 2;//alpha string
         }else if(isdigit(ch[0])){
            buf[c]=ch[0];
            buf[c+1]='\0';
            state = 3;//digit
          }else if(strcmp(ch," ")==0){
             state=-1;
          }else if(strcmp(ch,"\n")==0){
            state=-1;
            //line = line+1;
            it = 0;
          }
         break;
      case 1:// + - * / = // == , ; ( ) { }
          if(strcmp(ch," ")==0){
             state=0;

             if(strcmp(buf,"(")==0 ){            
                sc = sc+1;
                scope = sc;
             }else if(strcmp(buf,"}")==0){
                scope = scope-1;
             }
             tok_list[line].item[it].token=lookahead=FindTokV(buf);
             strcpy(tok_list[line].item[it].name ,buf);
             printf("This is state %d :%s\n",state,buf);
             tok_list[line].item[it].scope=scope;
             tok_list[line].line=line;
             tok_list[line].e_count=it;
             it = it+1;
          }else if(strcmp(ch,"\n")){
             state=0;
            
             if(strcmp(buf,"(")==0 ){
                sc = sc+1;
                scope = sc;
             }else if(strcmp(buf,"}")==0){
                scope = scope-1;
             }
             tok_list[line].item[it].token=lookahead=FindTokV(buf);
             strcpy(tok_list[line].item[it].name ,buf);
             tok_list[line].item[it].scope=scope;
             tok_list[line].line=line;
             tok_list[line].e_count=it;
             line = line+1;
             it = 0;
          }else if(strcmp(buf[c-1],"/")==0&&strcmp(ch,"/")==0){
           buf[c]=ch[0];
           buf[c+1]='\0';
           state = 1;
         }else if(strcmp(buf[c-1],"=")&&strcmp(ch,"=")==0){
           buf[c]=ch[0];
           buf[c+1]='\0';
           state = 1;
         }else state=100;//error
      break;
      case 2:// alpha string
         if(strcmp(ch," ")==0){
            state=0;
 //           printf("%s",buf);
            lookahead=FindTokV(buf);
            if(lookahead>=46||lookahead<40){//identifier
               //////////////
               //token_list//
               //////////////
               tok_list[line].item[it].token
                 =lookahead=FindTokV("id"); 
               strcpy(tok_list[line].item[it].name ,buf);
               tok_list[line].item[it].scope=scope;
               tok_list[line].line=line;
               tok_list[line].e_count=it;
               it = it+1;
               ////////////////
               //symbol table//
               ///////////////
             if(checkSymbolTable(buf)){
                strcpy(st[st_count].symbol,buf);
                st[st_count].token="id";
                  //tok_list[line].item[it].token;
                strcpy(st[st_count].type,tok_list[line].item[it-2].name);
                st[st_count].scope=scope;
                st_count = st_count+1;
             }

            }else{
            
               tok_list[line].item[it].token=lookahead; //Keyword
               strcpy(tok_list[line].item[it].name ,buf);
               tok_list[line].item[it].scope=scope;
               tok_list[line].line=line;
               tok_list[line].e_count=it;
               it = it+1;
            }
         }else if(isalpha(ch[0])||isdigit(ch[0])){
            buf[c]=ch[0];
            buf[c+1]='\0';
            state = 2;
         }
         break;
      case 3:
         if(strcmp(ch," ")==0){
            state=0;
            lookahead=FindTokV("num");
            
            tok_list[line].item[it].token = lookahead;
            strcpy(tok_list[line].item[it].name ,buf);
            tok_list[line].item[it].scope=scope;
            tok_list[line].line=line;
            tok_list[line].e_count=it;
            it = it+1;

         }else if(isdigit(ch[0])){
            buf[c]=ch[0];
            buf[c+1]='\0';
            state = 3;
         }
         break;
      case 99:
      //produce a lexeme
         
         break;
      case 100:
         //error
         break;
      default :
       break;
   }
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
   for(int i=0;i<g_size;i++){
      NonT[i].body[p_count].e_max = 0;
      NonT[i].p_max = 0;
   }
/*
   for(int i=0;i<g_size;i++){
      for(int j=0;j<10;j++){
         NonT[i].First.element[j]="non";
         NonT[i].Follow.element[j]="non";
      }
   }
   */
   for(int i=0;i<g_size;i++){
      NonT[i].First.set_max=0;
      NonT[i].Follow.set_max=0;
      for(int j=0;j<NonT[i].p_max;j++){
         NonT[i].body[j].First.set_max=0;
         NonT[i].body[j].Follow.set_max=0;
      }
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
               //init isNull of LHS
               NonT[nont_count].isNull = 5;
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
                //init isNull of RHS
               NonT[nont_count].body[p_count].isNull = 0;
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
   /////////////////
   //  Nullable
   //////////////
   for(int n=0;n<g_size;n++){
      NonT[n].isNull = Nullable(NonT[n].name);
   }
   ////////////////
   // First
   /////////////
   for(int c=0;c<20;c++){
      //RHS
    for(int n=0;n<g_size;n++){
         for(int b=0;b<NonT[n].p_max;b++){
            //     e1 U e2 if Nullable(e1) 
            //else e1
            for(int e=0;e<=NonT[n].body[b].e_max;e++){
              struct set SET=First(NonT[n].body[b].element[e]);
         
               if(SET.set_max==1&&strcmp(SET.element[0],"epsilon")==0){
                  //the first of body will be in the next element
               }
               else{
                  addSet(&NonT[n].body[b].First,SET);
                  printf("%d ",n);
                  break;
               }
            }  
         }
    }
      //LHS
    for(int n=0;n<g_size;n++){
       for(int b=0;b<NonT[n].p_max;b++){
          addSet(&NonT[n].First ,NonT[n].body[b].First);
       }
    }
   }
   //////////////
   //Follow
   ///////////
   struct set TMP_SET;
   TMP_SET.element[0]="$";
   TMP_SET.set_max = 1;
   addSet(&NonT[0].Follow ,TMP_SET);//Start symbol follow $

   for(int c=0;c<80;c++){
      for(int n=0;n<g_size;n++){
         Follow(NonT[n].name);
      }
   }

  //check data structure
   int loop_count = 0;
   for(;loop_count<g_size;loop_count++){
      printf("%s",NonT[loop_count].name);

      printf("\n    NULLABLE : ");
      printf("%d",NonT[loop_count].isNull);

      printf("\n    FIRST : ");
      for(int f=0;f<NonT[loop_count].First.set_max;f++){
         printf("%s ",NonT[loop_count].First.element[f]);
      }
      printf("\n    FOLLOW : ");
      for(int f=0;f<NonT[loop_count].Follow.set_max;f++){
         printf("%s ",NonT[loop_count].Follow.element[f]);
      }
      printf("\n");

      for(int p=0;p<=NonT[loop_count].p_max;p++){
         printf("\t");
         for(int e=0;e<=NonT[loop_count].body[p].e_max;e++){
            printf("%s  ",NonT[loop_count].body[p].element[e]);
         }
         printf("\n");
      }
      printf("\n");
   }
   //Nullable First Follow output_set
  FILE* output_set=fopen(outputfile_2,"w");
   fprintf(output_set,"Nullable\n");
   for(int n=0;n<g_size;n++){
      fprintf(output_set,"%s    :",NonT[n].name);
      if(NonT[n].isNull)fprintf(output_set,"True\n");
      else fprintf(output_set,"False\n");
   }
   fprintf(output_set,"\n");
   fprintf(output_set,"First\n\n");
   for(int n=0;n<g_size;n++){
      fprintf(output_set,"%s    :",NonT[n].name);
      for(int set_count=0;set_count<NonT[n].First.set_max;set_count++){
         fprintf(output_set,"%s ",NonT[n].First.element[set_count]);
      }
      fprintf(output_set,"\n");

   }
   fprintf(output_set,"\n");
   fprintf(output_set,"Follow\n\n");
   for(int n=0;n<g_size;n++){
      fprintf(output_set,"%s    :",NonT[n].name);
      for(int set_count=0;set_count<NonT[n].Follow.set_max;set_count++){
         fprintf(output_set,"%s ",NonT[n].Follow.element[set_count]);
      }
      fprintf(output_set,"\n");
   }

close(fd1);
close(fd3);
}
int main(){

   FILE* out;
   FILE* st_output;
   //build parsing table
   init();
   //init stack
   push(FindTokV("$"));
   push(0);//the value of start Symbol


   //open inputfile
   if((fd4=open(inputfile_1,O_RDONLY))==-1){
      printf("cannot open file.");
      exit(1);
   }
   //open outputfil
   //fd6=open(outputfile_1,O_CREAT|O_WRONLY,0644);
   out=fopen(outputfile_1,"w");
   st_output=fopen(outputfile_3,"w");

   //parsing
   int finish=0;
   int e_count=0;
   buf[0]='\0';
   while(!finish){
      state=-1;
    //  printf("lookahead is ");
      //buf[1]='\0';
      //give a token if success 
   while((fd5=read(fd4 ,chstr,chstr_size-1))>0){//fd6 bytes
      //check lexem and build token list ,symbol table 
      //and return lexem for parser
      //LL(1) parser
      chstr[1]='\0';
    //   printf("%s",chstr);
      if(state==-1)state=0;
   //lexer
      lexer(chstr,e_count);
      
      if(state==-1){
         e_count=0;
         buf[0]='\0';
      }else e_count = e_count+1;

      //write(fd6,chstr,chstr_size-1);
      if(state==100){
          printf("lexer analysis error!!");
          break;
       }else if(state==0){
          e_count=0;
          printf("%s",buf);
          lookahead=FindTokV(buf);
          printf("\ntoken is %d\n\n",lookahead);
          buf[0]='\0';
          break;
       }

   }
   e_count = 0;

      if(stp==0&&strcmp(stack[stp],"$")==0){
         finish=1; 
      }

      if(fd5<=0)finish=1;//for testing lexer
   }

   //////////////////////
   //symbol table output
   //////
   printf("SymbolTable\n");
   for(int sto=0;sto<st_count;sto++){
      printf("%s %d %s %d\n",st[sto].symbol,st[sto].token,st[sto].type,st[sto].scope);
      fprintf(st_output
        ,"%s %s %s %d\n",st[sto].symbol,st[sto].token,st[sto].type,st[sto].scope);
   }
 
   //token_list output
   int opt=0,tk=0;
   for(int l=0;l<line;l++){
      fprintf(out,"line %d:\n",l+1);
      printf("line %d:\n",l+1);
      for(int e=0;e<=tok_list[l].e_count;e++){
         tk=tok_list[l].item[e].token;
         if(tk<=46&&tk>=40)opt=0;
         else if(tk>=47&&tk<=60)opt=1;
         else if(tk>=61&&tk<=68)opt=2;
         else if(tk==69)opt=3;
         else if(tk==70)opt=4;
         else if(tk==71)opt=5;
         else if(tk==72)opt=6;
         else if(tk==73)opt=7;
         else if(tk==74)opt=8;
    //     printf("tk %d opt %d",tk,opt);
         fprintf(out,"    <%s>: %s\n",tok_sym[opt],tok_list[l].item[e].name);
         printf("    <%s>: %s\n",tok_sym[opt],tok_list[l].item[e].name);

      }
   }
   
   close(fd4);
   close(fd6);
   fclose(out);
   fclose(st_output);
}
