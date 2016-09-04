#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define maxsize 1001
#define reserveNum 6
#define true 1
#define false 0
typedef enum {
    ENDFILE, NUM, ID, ASSIGN, PLUS, MINUS, TIMES, DIVIDE, LT, LEQ, GT, GEQ, DEQ, NEQ, EQ,
    SEMI, COMMA, LBRACKET, RBRACKET, LSQUAB, RSQUAB, LBRACE, RBRACE, NOT, ERR,
    IF, ELSE, INT, RETURN, VOID, WHILE
}TokenType;
typedef enum {
    START, INNUM, INID, INASSIGN, DONE, INCOMMENT, OUTCOMMENT,INLEQ, INGEQ, INDEQ,INNEQ,
    
}StateType;
static struct {
    char* str;
    TokenType tok;
}reserveMap[maxsize] = {
    {"if", IF}, {"else", ELSE}, {"int", INT}, {"return", RETURN}, {"void", VOID}, {"while", WHILE}
};
TokenType checkReserve(char * s){
    int i;
    for(i=0;i<reserveNum;i++){
        if(!strcmp(s, reserveMap[i].str)) return reserveMap[i].tok;
    }
    return ID;
}
int isnum(char c){
    return (c<='9'&&c>='0');
}
int isid(char c){
    return ((c<='Z'&&c>='A') || (c<='z'&&c>='a'));
}
int backtrace = false;
char pre;
char tmpstring[maxsize];
int line=1;
void printToken(TokenType currentToken, char* s){
    switch (currentToken) {
        case NUM:
            printf("num: %s",s);
            break;
        case IF:
        case ELSE:
        case INT:
        case RETURN:
        case VOID:
        case WHILE:
            printf("reserve word: %s",s);
            break;
        case ID:
            printf("id: %s",s);break;
        case PLUS:
            printf("+");break;
        case MINUS:
            printf("-");break;
        case TIMES:
            printf("*");break;
        case DIVIDE:
            printf("//");break;
        case LT:
            printf("<");break;
        case LEQ:
            printf("<=");break;
        case GT:
            printf(">");break;
        case GEQ:
            printf(">=");break;
        case EQ:
            printf("=");break;
        case DEQ:
            printf("==");break;
        case NEQ:
            printf("!=");break;
        case SEMI:
            printf(";");break;
        case COMMA:
            printf(",");break;
        case LBRACKET:
            printf("(");break;
        case RBRACKET:
            printf(")");break;
        case LSQUAB:
            printf("[");break;
        case RSQUAB:
            printf("]");break;
        case LBRACE:
            printf("{");break;
        case RBRACE:
            printf("}");break;
    }
    printf("\n");
}
TokenType getToken()
{
    StateType state = START;
    TokenType currentToken;
    char c;
    int save=true;
    int top=0;
    while(state != DONE){
        if(!backtrace) c = getchar();
        backtrace = false;
        save = true;
        //        printf("%c,",c);
        if(c=='\377') return ENDFILE;
        switch (state) {
            case START:
                if(isdigit(c)) state = INNUM;
                else if(isid(c)) state = INID;
                else if(c == ' ' || c == '\t' || c== '\n') {save  = false;if(c=='\n') line++;}
                else if(c == '/') {
                    pre = getchar();
                    if(pre=='*') save = false, state = INCOMMENT;
                    else state = DONE, currentToken = DIVIDE, backtrace = true;
                }
                else {
                    state = DONE;
                    switch(c){
                        case EOF:
                            currentToken = ENDFILE;save=false;break;
                        case '+':
                            currentToken = PLUS;break;
                        case '-':
                            currentToken = MINUS;break;
                        case '*':
                            currentToken = TIMES;break;
                        case '>':
                            currentToken = GT;state=INGEQ; break;
                        case '<':
                            currentToken = LT;state=INLEQ; break;
                        case '=':
                            currentToken = EQ;state=INDEQ; break;
                        case '!':
                            currentToken = NOT;state=INNEQ; break;
                        case '(':
                            currentToken = LBRACKET;break;
                        case ')':
                            currentToken = RBRACKET;break;
                        case '[':
                            currentToken = LSQUAB;break;
                        case ']':
                            currentToken = RSQUAB;break;
                        case '{':
                            currentToken = LBRACE;break;
                        case '}':
                            currentToken = RBRACE;break;
                        case ',':
                            currentToken = COMMA;break;
                        case ';':
                            currentToken = SEMI;break;
                    }
                }
                break;
            case INCOMMENT:
                save = false;
                if(c=='*') {
                    pre = getchar();
                    if(pre=='/') state = START;
                    else if(pre==EOF) state = DONE, currentToken = ENDFILE;
                }
                break;
            case INNUM:
                if(!isnum(c))
                    save = false,
                    state = DONE,
                    currentToken = NUM,
                    backtrace = true;
                break;
            case INID:
                if(!isid(c))
                    save = false,
                    state = DONE,
                    currentToken = ID,
                    backtrace = true;
                break;
            case INGEQ:
                if(c=='=') state = DONE, currentToken=GEQ;
                else state=DONE, backtrace=true;
                break;
            case INLEQ:
                if(c=='=') state = DONE, currentToken=LEQ;
                else state=DONE, backtrace=true;
                break;
            case INDEQ:
                if(c=='=') state = DONE, currentToken=DEQ;
                else state=DONE, backtrace=true;
                break;
            case INNEQ:
                if(c=='=') state = DONE, currentToken=NEQ;
                else currentToken = ERR, backtrace = true;
        }
        if(save && top<maxsize)
            tmpstring[top++]=c;
        if(state == DONE){
            tmpstring[top++]='\0';
            if(currentToken == ID)
                currentToken = checkReserve(tmpstring);
        }
    }
    printf("line %d: ",line);
    printToken(currentToken, tmpstring);
    return currentToken;
}
int main()
{
    freopen("/Users/summer/Documents/360/写个浪漫这么南/写个浪漫这么南/test.txt", "r", stdin);
    while(getToken()!=ENDFILE);
    printf("end");
    return 0;
}