#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include "user.h"

//user(){
//    name="default";
//    uid=0;
//    money=0;
//}

int get_money(int usid){//узнать количество денег, имеющееся у пользователя
    int i=0;
    int res,tmp=0;
    int k=-1;
    char buffer[bufSize+1];
    FILE *file;
    char *fname="/home/user/project_t/money.txt";
    file = fopen(fname,"r");
    if(file == NULL)
    {
        perror("ERROR on openning file with money");
        exit(1);
    }
    bzero(buffer,bufSize+1);
    while(!feof(file)){
        fscanf(file,"%s", buffer);
        tmp=atoi(buffer);
        if(k==0){
            res=tmp;            
            break;
        }
        if(!(i%2)){
            if(usid==tmp)
                k=0;
        }
        i++;
    }
    fclose(file);
    return res;
}

int set_money(int uid, int value){
    char buf[15];
    int tmp=0;
    int before=0;
    int after=0;
    int spaces=0;
    fpos_t pos;
    FILE *file;
    char *fname="/home/user/project_t/money.txt";
    file = fopen(fname,"r+");
    if(file == NULL)
    {
        perror("ERROR on openning file with money");
        exit(1);
    }
    int k=1;
    int i = 0;
    while(fscanf(file,"%s", buf)!=EOF){
        if(k==0){
            before=ftell(file);
            fsetpos(file,&pos);
            fprintf(file,"\t%i",value);
            after=ftell(file);
            if(before>after) {
                spaces += before - after;
                while(spaces!=0){
                    fputc(' ', file);
                    spaces--;
                }
            }
            fflush(file);
            break;
        }
        fgetpos(file, &pos);
        if(!(i%2)){
            tmp=atoi(buf);
            if(tmp==uid){
                k=0;
            }
        }
        i++;
    }

}

int set_newid(){//задание нового id пользователя при регистрации
    int res=0;
    char uid[15];
    FILE *file;
    char *fname = "/home/user/project_t/us.txt";
    file = fopen(fname,"r");
    bzero(uid,sizeof(uid));
    if(file == NULL)
        {
            perror("ERROR on openning file with users");
            exit(1);
        }
    int i = 0;
    while(fscanf(file,"%s", uid)!=EOF){
        if(i%2){
            res=atoi(uid);
        }
        i++;
    }
    fclose(file);
    return res+1;
}
