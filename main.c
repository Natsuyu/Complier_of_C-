#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define maxsize 1001
#define reserveNum 6
#define true 1
#define false 0
#define SHOW_LEX true
#define BUFLEN 256
#define maxChild 3
#define size 211
#define SHIFT 4

static char lineBuf[BUFLEN]; /* holds the current line */
static int linepos = 0; /* current position in LineBuf */
static int bufsize = 0; /* current size of buffer string */
static int EOF_flag = false; /* corrects ungetNextChar behavior on EOF */
int EchoSource = false;
FILE * source;
int line=0;

typedef enum {
    ENDFILE, NUM, ID, ASSIGN, PLUS, MINUS, TIMES, DIVIDE, LT, LEQ, GT, GEQ, DEQ, NEQ, EQ,
    SEMI, COMMA, LBRACKET, RBRACKET, LSQUAB, RSQUAB, LBRACE, RBRACE, NOT, ERR,
    IF, ELSE, INT, RETURN, VOID, WHILE
}TokenType;
typedef enum {
    START, INNUM, INID, INASSIGN, DONE, INCOMMENT, OUTCOMMENT,INLEQ, INGEQ, INDEQ,INNEQ,
    
}StateType;
typedef enum {stmtK, expK, decK} nodeKind;
typedef enum {expstmtK, compdK, selectK, iteK, retK} stmtKind;
typedef enum {idK, constK, simpK, varK, opK, callK} expKind;
//varK to check the assign exp
//(id, num),(void, int),(simple, var = xx), op, call
typedef enum { intK, emptyK }varType;
typedef enum {fundK, vardK, arrK} decKind;    //变量和函数名放在符号表中

static struct {
    char* str;
    TokenType tok;
}reserveMap[reserveNum] = {
    {"if", IF}, {"else", ELSE}, {"int", INT}, {"return", RETURN}, {"void", VOID}, {"while", WHILE}
};
typedef struct node
{
    int lineno;
    int val;
    TokenType op;
    char* name;
    int childNum;
    decKind declaratkind;
    nodeKind nodekind;
    expKind expkind;
    stmtKind stmtkind;
    varType typ;
    struct node *child[maxChild], *silbing;
}node;
TokenType checkReserve(char * s);
int isnum(char c);
int isid(char c);
char* copyString(char * s);

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
char* copyString(char * s)
{   int n;
    char * t;
    if (s==NULL) return NULL;
    n = strlen(s)+1;
    t = (char*)malloc(n);
    if (t==NULL)
        printf("error");
    else strcpy(t,s);
    return t;
}
//step 1 declearation
int backtrace = false;
char tmpstring[maxsize];

//
static int getNextChar(void)
{ if (!(linepos < bufsize))
{
    line++;
    if (fgets(lineBuf,BUFLEN-1,source))
    { if (EchoSource) printf("%4d: %s",line,lineBuf);
        bufsize = strlen(lineBuf);
        linepos = 0;
        return lineBuf[linepos++];
    }
    else
    { EOF_flag = true;
        return EOF;
    }
}
else return lineBuf[linepos++];
}
static void ungetNextChar(void)
{ if (!EOF_flag) linepos-- ;}
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
            printf("/");break;
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
        c=getNextChar();
//        if(!backtrace) c = getchar();
//        backtrace = false;
        save = true;
//        if(c=='\377') return ENDFILE;
        switch (state) {
            case START:
                if(isnum(c)) state = INNUM;
                else if(isid(c)) state = INID;
                else if(c == ' ' || c == '\t' || c== '\n') {save  = false;}
                else if(c == '/') {
                    char pre=getNextChar();
                    if(pre=='*') save = false, state = INCOMMENT;
                    else state = DONE, currentToken = DIVIDE, ungetNextChar();
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
                    char pre = getNextChar();
                    if(pre=='/') state = START;
                    else if(pre==EOF) state = DONE, currentToken = ENDFILE;
                }
                break;
            case INNUM:
                if(!isnum(c))
                    save = false,
                    state = DONE,
                    currentToken = NUM,
//                    backtrace = true;
                    ungetNextChar();
                break;
            case INID:
                if(!isid(c))
                    save = false,
                    state = DONE,
                    currentToken = ID,
//                    backtrace = true;
                    ungetNextChar();
                break;
            case INGEQ:
                if(c=='=') state = DONE, currentToken=GEQ;
                else state=DONE, ungetNextChar();;
                break;
            case INLEQ:
                if(c=='=') state = DONE, currentToken=LEQ;
                else state=DONE, ungetNextChar();;
                break;
            case INDEQ:
                if(c=='=') state = DONE, currentToken=DEQ;
                else state=DONE, ungetNextChar();;
                break;
            case INNEQ:
                if(c=='=') state = DONE, currentToken=NEQ;
                else currentToken = ERR, ungetNextChar();;
        }
        if(save && top<maxsize)
            tmpstring[top++]=c;
        if(state == DONE){
            tmpstring[top++]='\0';
            if(currentToken == ID)
                currentToken = checkReserve(tmpstring);
        }
    }
    if(SHOW_LEX)
    {
        printf("line %d: ",line);
        printToken(currentToken, tmpstring);
    }
    return currentToken;
}
//<函数定义> -> <类型> <变量> ( <参数声明> ) { <函数块> }
//<变量定义> -> <类型> <变量> ( [<数组大小>] )
//function declaration
void varDec(node* tmp);
node* specid();
node* param();
node* funcDec();
node* stmt();
node* stmtList();
node* seleStmt();
node* iteraStmt();
node* retStmt();
node* exps();
node* epx();
node* simple_exp();
node* addexp();
node* term();
node* factor();
node* call();
node* var();
node* compdList();
node* Declarates();
node* declarationList();

node* createNode();
node* createNode();
void match(TokenType expected);
TokenType token;

void match(TokenType expected)
{
    if(expected == token) token=getToken();
    else {
        printf("now: %d, unexpected token -> ",expected);
        printToken(token,tmpstring);
    }
}
node* createNode()
{
    node* t = (node *)malloc(sizeof(node));
    if(!t) printf("no memory");
    else
    {
        t->lineno = 0;
        t->silbing = NULL;
        int i=0;
        for(i=0;i<maxChild;i++) t->child[i]=NULL;
    }
    return t;
}
node* declarationList()
{
    node *t = Declarates();
    node *p=t;
    while(token!=ENDFILE){
        node *q=Declarates();
        if(q)
        {
            if(!t) t=p=q;
            else
            {
                p->silbing=q;  //消除左递归
                p=q;
            }
        }
    }
    return t;
}
//type-specifier id
//return node{decK, vardK, varK, name}
node* specid()
{
    node* t = createNode(decK);
    t->declaratkind = vardK;    //it's a vardeclaration
    t->lineno = line;
    t->op = token;
    t->expkind = idK;
//    if(token==INT) t-= intK;
//    else if(token==VOID) t->typ=emptyK;
    
    match(token);
    t->name = copyString(tmpstring);
    match(ID);
    return t;
}
void varDec(node* tmp)  //if the var is arr, change the value of expkind to arrK
{
    switch(token)
    {
        case LSQUAB:
            tmp->declaratkind = arrK;
            match(LSQUAB);
            tmp->val = atoi(tmpstring);
            match(NUM);
            match(RSQUAB);
            match(SEMI);
            break;
        case SEMI:
            match(SEMI);
            break;
        default:
            printf("ERROR\n");
            break;
    }
}

node* param()      //match the param of function declaration
{
    node*t = specid();
    if(token == LSQUAB) {match(LSQUAB); match(RSQUAB);}
    return t;
}
//just match params
//return a linked list of params
node* funcDec()
{
    match(LBRACKET);
    node*t = NULL;
    if(token!=VOID)
    {
        node* t = param();
        node* p = t;
        while(token!=RBRACKET)
        {
            match(COMMA);
            node* q = param();
            p->silbing = q;
            p=q;
        }
    }
    else    //if param is void, define that there is no more params after that
    {
        t=createNode(expK);
        t->expkind = emptyK;
        match(VOID);
    }
    match(RBRACKET);
    return t;
}
node* stmt()
{
    node* q=NULL;
    switch(token)
    {
        case SEMI: q=exps();break;
        case LBRACE: q=compdList();break;
        case IF: q=seleStmt();break;
        case WHILE: q=iteraStmt();break;
        case RETURN: q=retStmt();break;
        default: q=exps();break;
    }
    return q;
}
//return a linked list of statement
node* stmtList()
{
    node* t = NULL;
    node* p = t;
    while(token != RBRACE)  //because statementlist only turn up in compound-stmt
    {
        node *q = stmt();
        if(q)  //q may be a null pointer
        {
            if(!t) t=p=q;
            else
            {
                p->silbing=q;
                p=q;
            }
        }
    }
    return t;
}

node* seleStmt()
{
    node* t = createNode(stmtK);
    match(IF);
    match(LBRACKET);
    t->child[0]=epx();
    match(RBRACKET);
    t->child[1]=stmt();
    if(token==ELSE)
    {
        match(ELSE);
        t->child[2]=stmt();
    }
    t->stmtkind = selectK;
    return t;
}
node* iteraStmt()
{
    node*t = createNode(stmtK);
    match(WHILE);
    match(LBRACKET);
    t->child[0]=epx();
    match(RBRACKET);
    t->child[1]=stmt();
    t->stmtkind = iteK;
    return t;
}
node* retStmt()
{
    node*t = createNode(stmtK);
    match(RETURN);
    if(token!=SEMI) t->child[0]=epx();
    match(SEMI);
    t->stmtkind = retK;
    return t;
}
node* exps()
{
    node* t = NULL;
    if(token==SEMI) match(token);
    else {t=epx();match(SEMI);}
    return t;
}
node* epx(void)
{
    node* t = NULL;
    t=simple_exp();
    if(t && t->expkind == varK) {   //in assign
        node*p = createNode(expK);
        p->expkind = opK;
        p->op = EQ;
        p->child[0] = t;
        t=p;
        t->child[1]=epx();
    }
    return t;
}
node* simple_exp()
{
    node* t = addexp();
    if(t->expkind==varK) return t;
    if(token==LT || token==LEQ || token==GT || token==GEQ || token==DEQ || token==NEQ)
    {
        node*p = createNode(expK);
        p->op=token;
        match(token);
        p->child[0]=t;
        t=p;
        t->child[1]=addexp();
    }
    return t;
}
node* addexp()
{
    node* t = term();
    if(t->expkind==varK) return t;
    while(token==PLUS || token==MINUS)
    {
        node* p = createNode(expK);
        p->op = token;
        match(token);
        p->child[0]=t;
        t=p;
        t->child[1]=term();
    }
    return t;
}
node* term()
{
    node* t = factor();
    if(token==EQ)
    {
        t->expkind=varK;
        match(EQ);
        return t;
    }
    while(token==TIMES || token==DIVIDE)
    {
        node* p = createNode(expK);
        p->op=token;
        match(token);
        p->child[0]=t;
        t=p;
        t->child[1]=factor();
    }
    return t;
}
node* factor()
{
    node* t = NULL;
    switch (token) {
        case NUM:
            t=createNode(expK);
            t->expkind = constK;
            t->val=atoi(tmpstring);
            match(NUM);
            break;
        case LBRACKET:
            match(LBRACKET);
            t=epx();
            match(RBRACKET);
            break;
        case ID:
            t=createNode(expK);
            t->expkind = idK;
            t->name = copyString(tmpstring);
            match(ID);
            if(token==LBRACKET)     //it's call
            {
                t->expkind = callK;
                t->child[0]=call();
                match(RBRACKET);
            }
            else if(token==LSQUAB)  //arr
            {
                match(LSQUAB);
                node* p = createNode(expK);    //pay attention about the type of node
                p->declaratkind = arrK;
                p->name = t->name;
                p->child[0]=epx();
                t=p;
                match(RSQUAB);
            }
            break;
        default:
            printf("ERROR\n");
            break;
    }
    return t;
}
node* call()
{
    match(LBRACKET);
    node* t = NULL;
    if(token==RBRACKET) return t;
    t=epx();
    node*p=t;
    while(token==COMMA)
    {
        match(COMMA);
        node* q=epx();
        p->silbing=q;
        p=q;
    }
    return t;
}
node* var()
{
    node* t= createNode(constK);
    t->name = copyString(tmpstring);
    match(ID);
    if(token==LSQUAB)
    {
        match(LSQUAB);
        node* p = createNode(constK);    //pay attention about the type of node
        p->expkind = arrK;
        p->child[0]=t;
        p->child[1]=epx();
        return p;
    }
    return t;
}
node* compdList()
{
    match(LBRACE);
    node* ret = createNode(stmtK);
    node* t = NULL;
    node* p = t;
    if(token!=RBRACE)
    {
        while(token == INT || token == VOID)
        {
            node*q=specid();
            varDec(q);
            if(!t) t=p=q;
            else
            {
                p->silbing=q;
                p=q;
            }
        }
        ret->child[0]=t;   //local declaration
        if(token!=RBRACE)
        {
            ret->child[1]=stmtList();   //statment list
        }
    }
    match(RBRACE);
    ret->stmtkind = compdK;
    return ret;
}
node* Declarates()
{
    node* t = createNode(decK);
    node* p = specid();
    t->child[0]=p;
    switch (token) {
        case SEMI:
        case LSQUAB:
            varDec(p);
            t=p;
            break;
        case LBRACKET:
            t->child[1]=funcDec();
            t->child[2]=compdList();
            break;
        default:
            printf("ERROR\n");
            break;
    }
    return t;
}
node* parse()
{
    token=getToken();
    
    node* t=declarationList();
    return t;
}

//step 3
typedef struct bucketList
{
    int lineno;
    char *name;
    int loc;
    TokenType typ;
    struct bucketList * next;
}bucket;
bucket* hs_table[size];
int hash(char*s)
{
    int i=0,tmp=0;
    while(s[i]!='\0')
    {
        tmp=((tmp<<SHIFT) + s[i]) % size;
        i++;
    }
    return tmp;
}
void insertNode(node*);
void nullProc(node*);
void append(node* t, int loc);
bucket* createBucNode(node*t);

void travel(node *x, void(* preProc) (node*), void(* postProc)(node*))
{
    if(!x) return;
    preProc(x);
    int i;
    for(i=0;i<maxChild;i++)
        travel(x->child[i], preProc, postProc);
    
    travel(x->silbing, preProc, postProc);
    postProc(x);
}
void nullProc(node* t){return;}
void insertNode(node* t)
{
    if(!t) return;
    if(t->expkind == idK)
    {
        int loc = hash(t->name);
        append(t, loc);
    }
}
bucket* createBucNode(node*t)
{
    bucket*x = (bucket*)malloc(sizeof(bucket));
    x->lineno = t->lineno;
    x->name = t->name;
    x->typ = t->op;
    x->next = NULL;
    return x;
}
void append(node* t, int loc)
{
    bucket* x = createBucNode(t);
    bucket* head = hs_table[loc];
    if(!head) hs_table[loc] =x;
    else
    {
        while(head && head->next) head=head->next;
        head->next=x;
    }
}
void outSympol()
{
    for(int i=0;i<size;i++)
    {
        if(hs_table[i])
        {
            printf("%s\t%d\t",hs_table[i]->name, hs_table[i]->typ);
            bucket *p = hs_table[i];
            while(p)
            {
                printf("%d ",p->lineno);
                p=p->next;
            }
        }
    }
}
void init()
{
    int i;
    for(i=0;i<size;i++) hs_table[i]=NULL;
}
int main()
{
    init();
    source = fopen("/Users/summer/Documents/360/写个浪漫这么南/写个浪漫这么南/test.txt","r");
//        while(getToken()!=ENDFILE);
    node* root = parse();
    travel(root, insertNode, nullProc);
    outSympol();
    printf("end\n");
    return 0;
}