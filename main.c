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

typedef enum {stmtK, decK} nodeKind;
typedef enum {expstmtK, compdK, selectK, iteK, retK} stmtKind;
typedef enum {opK, assignK, idK, constK, callK} expKind;
typedef enum {fundK, vardK, arrK } decKind;

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
    decKind deckind;
    nodeKind nodekind;
    expKind expkind;
    stmtKind stmtkind;
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
        t->name = NULL;
        t->lineno = 0;
        t->stmtkind = expstmtK;
        t->expkind = opK;
        t->nodekind = stmtK;
        t->deckind = vardK;
        t->op = ENDFILE;
        t->val = 0;
        int i=0;
        for(i=0;i<maxChild;i++) t->child[i]=NULL;t->silbing = NULL;
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
    node* t = createNode();

    t->lineno = line;
    t->op = token;      //int or void
    t->deckind = vardK;
    
    match(token);
    t->name = copyString(tmpstring);
    match(ID);
//    printf("id:%s op:%d line:%s\n",t->name, t->op, t->lineno);
    return t;
}
void varDec(node* tmp)  //if the var is arr, change the value of expkind to arrK
{
    switch(token)
    {
        case LSQUAB:
            tmp->deckind = arrK;
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
    if(token == LSQUAB) {match(LSQUAB); match(RSQUAB);t->deckind = arrK;}
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
        t = param();
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
        t=createNode();
        t->deckind = vardK;
        t->op = token;
        match(VOID);
    }
    match(RBRACKET);
    printf("%d\n",t);
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
    q->nodekind = stmtK;
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
    node* t = createNode();
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
    node*t = createNode();
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
    node*t = createNode();
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
    t->stmtkind = expstmtK;
    return t;
}
node* epx(void)
{
    node* t = NULL;
    t=simple_exp();
    if(t && t->expkind == assignK) {   //in assign
        node*p = createNode();
        p->expkind = opK; //? 待定
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
    if(t->expkind==assignK) return t;
    if(token==LT || token==LEQ || token==GT || token==GEQ || token==DEQ || token==NEQ)
    {
        node*p = createNode();
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
    if(t->expkind==assignK) return t;
    while(token==PLUS || token==MINUS)
    {
        node* p = createNode();
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
        t->expkind=assignK;
        match(EQ);
        return t;
    }
    while(token==TIMES || token==DIVIDE)
    {
        node* p = createNode();
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
            t=createNode();
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
            t=createNode();
            t->expkind = idK;
            t->name = copyString(tmpstring);
            t->lineno = line;
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
                t->deckind = arrK;
                t->child[0]=epx();
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

node* compdList()
{
    match(LBRACE);
    node* ret = createNode();
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
    node* t = createNode();
    node* p = specid();
    t->child[0]=p;
    t->nodekind = decK;
    switch (token) {
        case SEMI:
        case LSQUAB:
            varDec(p);
            t=p;
            break;
        case LBRACKET:
            t->child[1]=funcDec();
            t->child[2]=compdList();
            t->deckind = fundK;
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
    match(ENDFILE);
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
        tmp= ((tmp<<SHIFT) + s[i]) % size;
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
    if(!t || !t->name) return;
    if(t->name)
    {
//        printf("%s\n",t->name);
    }
    if(t->expkind == idK || t->deckind == vardK || t->deckind == arrK)
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
        if(head && !head->typ) head->typ = x->typ;
        while(head && head->next) head=head->next;
        if(head->lineno != x->lineno) head->next=x;
    }
}
void outSympol()
{
    for(int i=0;i<size;i++)
    {
        if(hs_table[i])
        {
            printf("%-10s%-8d\t",hs_table[i]->name, hs_table[i]->typ);
            bucket *p = hs_table[i];
            while(p)
            {
                printf("%d ",p->lineno);
                p=p->next;
            }
            printf("\n");
        }
    }
}
void init()
{
    int i;
    for(i=0;i<size;i++) hs_table[i]=NULL;
}
//step 4
void genExp(node*t,int dep);
void genStmt(node*t);
void genDec(node*t);
void genCode(node*t, int level);

void genExp(node*t,int dep)
{
    if(!t) return;
    if(!t->child[0] && !t->child[1])
    {
        switch (t->expkind) {
            case idK:
                printf("%s ",t->name);
                break;
            case constK:
                printf("%d ",t->val);
            default:
                break;
        }
    }
    printf("t%d = ",dep);
    genExp(t->child[0],dep+1);
    switch (t->op) {
        case EQ: printf("=");break;
        case PLUS: printf("+");break;
        case MINUS: printf("-");break;
        case TIMES: printf("*");break;
        case DIVIDE: printf("/");break;
        case LT: printf("<");break;
        case GT: printf(">");break;
        case LEQ: printf("<=");break;
        case GEQ: printf(">=");break;
        case NEQ: printf("!=");break;
        case DEQ: printf("==");break;
    }
    genExp(t->child[1], dep+2);
    
}
void genStmt(node*t)
{
    expKind tmp;
    int ret =0;
    switch (t->stmtkind) {
        case expstmtK:
            ret = genExp(t,0);
        default:
            break;
    }
}
void genDec(node*t)
{
    switch (t->deckind) {
        case vardK:
            printf("assign %s\n",t->name);
            break;
        case arrK:
            printf("assign %s[%d]\n",t->name,t->val);
        case fundK:
            printf("assign Function %s, params:\n",t->child[0]->name);
            genCode(t->child[1], 0);
            genStmt(t->child[2]);
        default:
            break;
    }
}
void genCode(node*t, int level)
{
    switch (t->nodekind) {
        case decK:
            genDec(t);
            break;
        case stmtK:
            genStmt(t);
            break;
    }
    genCode(t->silbing, 0);
}
int main()
{
    init();
    source = fopen("/Users/summer/Documents/360/写个浪漫这么南/写个浪漫这么南/test.txt","r");
//        while(getToken()!=ENDFILE);
    node* root = parse();
    travel(root, insertNode, nullProc);
    outSympol();
    genCode(root,0);
    printf("end\n");
    return 0;
}