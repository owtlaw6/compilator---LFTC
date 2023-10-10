#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>


//cod pentru enum + structura Token
enum{ID, BREAK, CHAR, DOUBLE, ELSE, FOR, IF, INT, RETURN, STRUCT, VOID, WHILE, 
CT_INT, CT_REAL, CT_CHAR, CT_STRING, COMMA, SEMICOLON, LPAR, RPAR, LBRACKET,
RBRACKET, LACC, RACC, END, ADD, SUB, MUL, DIV, DOT, AND, OR, NOT, ASSIGN, EQUAL,
NOTEQ, LESS, LESSEQ, GREATER, GREATEREQ};

typedef struct _Token{
    int code; //codul (numele)
    union{
        char *text; //folosit pt ID, CT_STRING (alocat dinamic)
        int i; //folosit pt CT_INT, CT_CHAR
        double r; //folosit pt CT_REAL
    };
    int line; //linia din fisierul de intrare
    struct _Token *next; //inlantuire la urmatorul AL
}Token;

//functie pentru erori
void err(const char* fmt,...){
    va_list va;
    va_start(va,fmt);
    fprintf(stderr,"error: ");
    fprintf(stderr,fmt,va);
    fputc('\n',stderr);
    va_end(va);
    exit(-1);
}

//functia pt errori pe token
void tkerr(const Token *tk, const char *fmt,...){
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error in line %d ", tk->line);
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}

//definire alocare
#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL)err("notenough memory");

//variabile globale
Token *tokens = NULL;
Token *lastToken; //va pointa la ultimul AL din lista
char inBuff[30001];
int line = 1;
char *pCrtCh;
char *pStartCh_s; //inceputul de string
int final_int = 0;
int real_count = 0;
char *startE = NULL;
double x;



//Adaugarea de Token
Token *addTk(int code){
    Token *tk;
    SAFEALLOC(tk, Token);
    tk->code = code;
    tk->line = line;
    tk->next = NULL;
    if(lastToken){
        lastToken->next=tk;
    }
    else{
        tokens=tk;
    }
    lastToken=tk;
    return tk;
}

//functia pentru crearea string-ului pt ID
char *createString(const char *pStartCh, char *pCrtCh){
    char *str = NULL;
    if((str = (char*) malloc ((pCrtCh - pStartCh) * sizeof(char))) == NULL)
        err("not enough memory");
    strncpy(str, pStartCh, pCrtCh - pStartCh);
    str[pCrtCh - pStartCh] = '\0';
    return str;
}

//functia pentru nextToken
int getNextToken(){
    int state=0, nCh;
    char ch;
    const char *pStartCh;
    Token *tk;
    while(1){
        ch=*pCrtCh;
        switch(state){  //switch pe starile DT
            case 0:
                if(isdigit(ch)){  //CT_INT, CT_REAL
                    pStartCh=pCrtCh;
                    pCrtCh++;
                    startE = pCrtCh - 1;
                    final_int = ch - '0'; //ia valoarea int
                    state = 1;
                }
                else if(ch == '\''){  //CT_CHAR
                    pCrtCh++;
                    state = 12;
                }
                else if(ch == '"'){  //CT_STRING
                    pStartCh_s = pCrtCh++;
                    state = 15;
                }
                else if(ch == ','){  //COMMA
                    pCrtCh++;
                    state = 17;
                }
                else if(ch == ';'){  //SEMICOLON
                    pCrtCh++;
                    state = 18;
                }
                else if(ch == '('){  //LPAR
                    pCrtCh++;
                    state = 19;
                }
                else if(ch == ')'){  //RPAR
                    pCrtCh++;
                    state = 20;
                }
                else if(ch == '['){  //LBRACKET
                    pCrtCh++;
                    state = 21;
                }
                else if(ch == ']'){  //RBRACKET
                    pCrtCh++;
                    state = 22;
                }
                else if(ch == '{'){  //LACC
                    pCrtCh++;
                    state = 23;
                }
                else if(ch == '}'){  //RACC
                    pCrtCh++;
                    state = 24;
                }
                else if(ch == '\0'){  //END
                    pCrtCh++;
                    state = 25;
                }
                else if(ch == '+'){  //ADD
                    pCrtCh++;
                    state = 26;
                }
                else if(ch == '-'){  //SUB
                    pCrtCh++;
                    state = 27;
                }
                else if(ch == '*'){  //MUL
                    pCrtCh++;
                    state = 28;
                }
                else if(ch == '/'){  //DIV
                    pCrtCh++;
                    state = 29;
                }
                else if(ch == '.'){  //DOT
                    pCrtCh++;
                    state = 32;
                }
                else if(ch == '&'){  //AND
                    pCrtCh++;
                    state = 33;
                }
                else if(ch == '|'){  //OR
                    pCrtCh++;
                    state = 35;
                }
                else if(ch == '!'){  //NOT, NOTEQ
                    pCrtCh++;
                    state = 37;
                }
                else if(ch == '='){  //ASSIGN, EQUAL
                    pCrtCh++;
                    state = 40;
                }
                else if(ch == '<'){  //LESS, LESSEQ
                    pCrtCh++;
                    state = 43;
                }
                else if(ch == '>'){ //GREATER, GREATEREQ
                    pCrtCh++;
                    state = 46;
                }
                else if((isalpha(ch) || ch == '_')){  //ID
                    pStartCh = pCrtCh++;
                    state = 49;
                }
                else if(ch == ' ' || ch == '\t' || ch == '\r')  //SPACE
                    pCrtCh++;
                else if(ch == '\n'){
                    line++;
                    pCrtCh++;
                }
                else if(ch == 0){
                    tk = addTk(END);
                    return END;
                }
                else tkerr(addTk(END), "Caracter extraterestru!\n");
                break;
            case 1:
                if(isdigit(ch)){
                    pCrtCh++;
                    final_int = final_int * 10 + (ch - '0');
                }
                else if(ch == '.'){ //CT_REAL
                    pCrtCh++;
                    state = 3;
                }
                else if(ch == 'e' || ch == 'E'){ //CT_REAL
                    pCrtCh++;
                    state = 8;
                }
                else state = 2; //CT_INT
                break;
            case 2: //CT_INT final
                tk = addTk(CT_INT);
                tk->i = final_int;
                final_int = 0;
                return CT_INT;
            case 3: //CT_REAL
                if(isdigit(ch)){
                    pCrtCh++;
                    final_int = final_int * 10 + (ch - '0');
                    real_count++; //incepem numararea nr de zecimale pt partea reala
                    state = 4;
                }
                else err("Nu am primit zecimale!");
                break;
            case 4: //CT_REAL
                if(isdigit(ch)){
                    pCrtCh++;
                    final_int = final_int * 10 + (ch - '0');
                    real_count++; //incepem numararea nr de zecimale pt partea reala
                } 
                else if(ch == 'e' || ch == 'E'){
                    pCrtCh++;
                    state = 5;
                }
                else state = 11;
                break;
            case 5: //CT_REAL
                if(ch == '+' || ch == '-')  pCrtCh++;
                state = 6;
                break;
            case 6: //CT_REAL
                if(isdigit(ch)){
                    pCrtCh++;
                    state = 7;
                }
                break;
            case 7: //CT_REAL
                if(isdigit(ch)) pCrtCh++;
                else{
                    char *stopstring;
                    x = strtod(startE, &stopstring);
                    state = 11;
                }
                break;
            case 8: //CT_REAL
                if(ch == '+' || ch == '-')  pCrtCh++;
                state = 9;
                break;
            case 9: //CT_REAL
                if(isdigit(ch)){
                    pCrtCh++;
                    state = 10;
                }
                break;
            case 10: //CT_REAL
                if(isdigit(ch)) pCrtCh++;
                else{
                    char *stopstring;
                    x = strtod(startE, &stopstring);
                    state = 11;
                }
                break;
            case 11: //CT_REAL final
                if(x){
                    tk = addTk(CT_REAL); 
                    tk->r = x;
                    x = 0;
                    final_int = real_count = 0;
                    return CT_REAL;
                }
                else{
                    tk = addTk(CT_REAL);
                    double d = final_int;
                    for(int k = 0; k < real_count; k++){
                        d /= 10;
                    }
                    tk->r = d;
                    final_int = real_count = 0;
                    return CT_REAL;
                }
                
            case 12: //CT_CHAR
                if(ch != '\''){
                    pCrtCh++;
                    state = 13;
                }
                break;
            case 13: //CT_CHAR
                if(ch == '\''){
                    pCrtCh++;
                    state = 14;
                }
                break;
            case 14: //CT_CHAR final
                tk = addTk(CT_CHAR);
                tk->i = *(pCrtCh - 2);
                return CT_CHAR;
            case 15: //CT_STRING
                if(ch != '"') pCrtCh++;
                if(ch == '"'){
                    pCrtCh++;
                    state = 16;
                }
                break;
            case 16: //CT_STRING final
                tk = addTk(CT_STRING);
                tk->text = createString(pStartCh_s, pCrtCh);
                return CT_STRING;
            case 17: //COMMA final
                tk = addTk(COMMA);
                return COMMA;
            case 18: //SEMICOLON final
                tk = addTk(SEMICOLON);
                return SEMICOLON;
            case 19: //LPAR final
                tk = addTk(LPAR);
                return LPAR;
            case 20: //RPAR final
                tk = addTk(RPAR);
                return RPAR;
            case 21: //LBRACKET final
                tk = addTk(LBRACKET);
                return LBRACKET;
            case 22: //RBRACKET final
                tk = addTk(RBRACKET);
                return RBRACKET;
            case 23: //LACC final
                tk = addTk(LACC);
                return LACC;
            case 24: //RACC final
                tk = addTk(RACC);
                return RACC;
            case 25: //END final
                tk = addTk(END);
                return END;
            case 26: //ADD final
                tk = addTk(ADD);
                return ADD;
            case 27: //SUB final
                tk = addTk(SUB);
                return SUB;
            case 28: //MUL final
                tk = addTk(MUL);
                return MUL;
            case 29: //DIV, LINECOMMENT
                if(ch == '/'){
                    pCrtCh++;
                    state = 31;
                }
                else state = 30;
                break;
            case 30: //DIV final
                tk = addTk(DIV);
                return DIV;
            case 31: //LINECOMMENT
                if(ch != '\t' && ch != '\r' && ch != '\n') pCrtCh++;
                else state = 0;
                break;
            case 32: //DOT final
                tk = addTk(DOT);
                return DOT;
            case 33: //AND
                if(ch == '&'){
                    pCrtCh++;
                    state = 34;
                }
                break;
            case 34: //AND final
                tk = addTk(AND);
                return AND;
            case 35: //OR
                if(ch == '|'){
                    pCrtCh++;
                    state = 36;
                }
                break;
            case 36: //OR final
                tk = addTk(OR);
                return OR;
            case 37: //NOT, NOTEQ
                if(ch == '='){
                    pCrtCh++;
                    state = 39;
                }
                else state = 38;
                break;
            case 38: //NOT final
                tk = addTk(NOT);
                return NOT;
            case 39: //NOTEQ final
                tk = addTk(NOTEQ);
                return NOTEQ;
            case 40: //ASSIGN, EQUAL
                if(ch == '='){
                    pCrtCh++;
                    state = 42;
                }
                else state = 41;
                break;
            case 41: //ASSIGN final
                tk = addTk(ASSIGN);
                return ASSIGN;
            case 42: //EQUAL final
                tk = addTk(EQUAL);
                return EQUAL;
            case 43: //LESS, LESSEQ
                if(ch == '='){
                    pCrtCh++;
                    state = 45;
                }
                else state = 44;
                break;
            case 44: //LESS final
                tk = addTk(LESS);
                return LESS;
            case 45: //LESSEQ final
                tk = addTk(LESSEQ);
                return LESSEQ;
            case 46: //GREATER, GREATEREQ
                if(ch == '='){
                    pCrtCh++;
                    state = 48;
                }
                else state = 47;
                break;
            case 47: //GREATER final
                tk = addTk(GREATER);
                return GREATER;
            case 48: //GREATEREQ final
                tk = addTk(GREATEREQ);
                return GREATEREQ;
            case 49: //ID
                if(isalnum(ch) || ch == '_') pCrtCh++;
                else state = 50;
                break;
            case 50: //ID final
                nCh = pCrtCh - pStartCh; //lungime cuvant gasit
                if(nCh == 5 && !memcmp(pStartCh,"break", 5)) tk = addTk(BREAK);
                else if(nCh == 4 && !memcmp(pStartCh, "char", 4)) tk = addTk(CHAR);
                else if(nCh == 6 && !memcmp(pStartCh, "double", 6)) tk = addTk(DOUBLE);
                else if(nCh == 4 && !memcmp(pStartCh, "else", 4)) tk = addTk(ELSE);
                else if(nCh == 3 && !memcmp(pStartCh, "for", 3)) tk = addTk(FOR);
                else if(nCh == 2 && !memcmp(pStartCh, "if", 2)) tk = addTk(IF);
                else if(nCh == 3 && !memcmp(pStartCh, "int", 3)) tk = addTk(INT);
                else if(nCh == 6 && !memcmp(pStartCh, "return", 6)) tk = addTk(RETURN);
                else if(nCh == 6 && !memcmp(pStartCh, "struct", 6)) tk = addTk(STRUCT);
                else if(nCh == 4 && !memcmp(pStartCh, "void", 4)) tk = addTk(VOID);
                else if(nCh == 5 && !memcmp(pStartCh, "while", 5)) tk = addTk(WHILE);
                else{
                    tk = addTk(ID);
                    tk->text = createString(pStartCh, pCrtCh);
                }
                return tk->code;
            default:
                err("Caracter Extraterestru!");
                break;
        }
    }
}

//afisarea bazata pe codurile enum-ului
void afisare(Token *tk){
    int code = tk->code;
    switch(code){
        case 0: 
            printf("ID:%s ", tk->text); 
            break;
        case 1: 
            printf("BREAK ");
            break;
        case 2: 
            printf("CHAR "); 
            break;
        case 3: 
            printf("DOUBLE "); 
            break;
        case 4: 
            printf("ELSE ");
            break;
        case 5: 
            printf("FOR "); 
            break;
        case 6: 
            printf("IF "); 
            break;
        case 7: 
            printf("INT ");
            break;
        case 8: 
            printf("RETURN ");
            break;
        case 9: 
            printf("STRUCT ");
            break;
        case 10: 
            printf("VOID ");
            break;
        case 11: 
            printf("WHILE ");
            break;
        case 12: 
            printf("CT_INT:%d ", tk->i);
            break;
        case 13: 
            printf("CT_REAL:%f ", tk->r);
            break;
        case 14: 
            printf("CT_CHAR:%c ", tk->i);
            break;
        case 15: 
            printf("CT_STRING:%s ", tk->text);
            break;
        case 16: 
            printf("COMMA ");
            break;
        case 17: 
            printf("SEMICOLON ");
            break;
        case 18: 
            printf("LPAR ");
            break;
        case 19: 
            printf("RPAR ");
            break;
        case 20: 
            printf("LBRACKET ");
            break;
        case 21: 
            printf("RBRACKET ");
            break;
        case 22: 
            printf("LACC ");
            break;
        case 23: 
            printf("RACC ");
            break;
        case 24: 
            printf("END ");
            break;
        case 25: 
            printf("ADD ");
            break;
        case 26: 
            printf("SUB ");
            break;
        case 27: 
            printf("MUL ");
            break;
        case 28: 
            printf("DIV ");
            break;
        case 29: 
            printf("DOT ");
            break;
        case 30: 
            printf("AND ");
            break;
        case 31: 
            printf("OR ");
            break;
        case 32: 
            printf("NOT ");
            break;
        case 33:
            printf("ASSIGN ");
            break;
        case 34: 
            printf("EQUAL ");
            break;
        case 35: 
            printf("NOTEQ ");
            break;
        case 36: 
            printf("LESS ");
            break;
        case 37: 
            printf("LESSEQ ");
            break;
        case 38: 
            printf("GREATER ");
            break;
        case 39: 
            printf("GREATEREQ ");
            break;
    }
}

//afisarea atomilor sub forma: linie ATOM
void showAtoms(){
    Token *token;
    token = tokens;
    while(token != lastToken){
        printf("%d ",token->line);
        afisare(token);
        printf("\n");
        token = token->next;
    }
    printf("%d ", token->line);
    afisare(token);
}

/*
--------------------ASIN----------------------
*/
//variabile globale ASIN
Token *iTk;
Token *consumedTk;

/*
----------------Analiza de Domeniu-------------------
*/

struct Symbol;
typedef struct Symbol Symbol;

typedef enum {		// tipul de baza
    TB_INT, TB_DOUBLE, TB_CHAR, TB_VOID, TB_STRUCT
}TypeBase;

typedef struct {		// tipul unui simbol
    TypeBase tb;
    Symbol* s;
    int n;
}Type;

int typeSize(Type* t);

typedef enum{		// felul unui simbol
	SK_VAR,SK_PARAM,SK_FN,SK_STRUCT
	}SymKind;

struct Symbol{
	const char *name;		// numele simbolului. Simbolul nu detine acest pointer, ci el este alocat in alta parte (ex: in Token)
	SymKind kind;		// felul
	Type type;		// tipul
	// owner:
	//		- NULL pentru simboluri globale
	//		- o structura pentru variabile definite in acea structura
	//		- o functie pentru parametri/variabile locale acelei functii
	Symbol *owner;
	Symbol *next;		// inlantuire la urmatorul simbol din lista
	union{		// date specifice pentru fiecare fel de simbol
		// indexul in fn.locals pentru variabile locale
		// indexul in globals pentru variabile globale
		// indexul in structura pentru membri de structura
		int varIdx;
		// indexul in fn.params pentru parametri
		int paramIdx;
		// membrii unei structuri
		Symbol *structMembers;
		struct{
			Symbol *params;		// parametrii functiei
			Symbol *locals;		// variabilele locale, inclusiv cele din subdomeniile fn
			void(*extFnPtr)();		// !=NULL pentru functii externe
			int instrIdx;
			}fn;
		};
	};
Symbol* newSymbol(const char* name, SymKind kind);
Symbol* dupSymbol(Symbol* symbol);
Symbol* addSymbolToList(Symbol** list, Symbol* s);
int symbolsLen(Symbol* list);
void freeSymbol(Symbol* s);

typedef struct _Domain {
    struct _Domain* parent;
    Symbol* symbols;
}Domain;

extern Domain* symTable;

Domain* pushDomain();
void dropDomain();
void showDomain(Domain* d, const char* name);
Symbol* findSymbolInDomain(Domain* d, const char* name);
Symbol* findSymbol(const char* name);
Symbol* addSymbolToDomain(Domain* d, Symbol* s);

extern char* globalMemory;
extern int nGlobalMemory;

int allocInGlobalMemory(int nBytes);

Domain* symTable = NULL;
char* globalMemory = NULL;
int nGlobalMemory = 0;
Symbol* owner = NULL;

int typeBaseSize(Type *t){
	switch(t->tb){
		case TB_INT:return sizeof(int);
		case TB_DOUBLE:return sizeof(double);
		case TB_CHAR:return sizeof(char);
		case TB_VOID:return 0;
		default:{		// TB_STRUCT
			int size=0;
			for(Symbol *m=t->s->structMembers;m;m=m->next){
				size+=typeSize(&m->type);
				}
			return size;
			}
		}
	}

int typeSize(Type *t){
	if(t->n<0)return typeBaseSize(t);
	if(t->n==0)return sizeof(void*);
	return t->n*typeBaseSize(t);
	}

// elibereaza memoria ocupata de o lista de simboluri
void freeSymbols(Symbol *list){
	for(Symbol *next;list;list=next){
		next=list->next;
		freeSymbol(list);
		}
	}

Symbol *newSymbol(const char *name,SymKind kind){
	Symbol *s;
	SAFEALLOC(s,Symbol)
	// seteaza pe 0/NULL toate campurile
	memset(s,0,sizeof(Symbol));
	s->name=name;
	s->kind=kind;
	return s;
	}

Symbol *dupSymbol(Symbol *symbol){
	Symbol *s;
	SAFEALLOC(s,Symbol)
	*s=*symbol;
	s->next=NULL;
	return s;
	}

// adauga s la sfarsitul listei list
// s->next este NULL de la newSymbol
Symbol *addSymbolToList(Symbol **list,Symbol *s){
	Symbol *iter=*list;
	if(iter){
		while(iter->next)iter=iter->next;
		iter->next=s;
		}else{
		*list=s;
		}
	return s;
	}

int symbolsLen(Symbol *list){
	int n=0;
	for(;list;list=list->next)n++;
	return n;
	}

void freeSymbol(Symbol *s){
	switch(s->kind){
		case SK_FN:
			freeSymbols(s->fn.params);
			freeSymbols(s->fn.locals);
			break;
		case SK_STRUCT:
			freeSymbols(s->structMembers);
			break;
		}
	free(s);
	}

Domain *pushDomain(){
	Domain *d;
	SAFEALLOC(d,Domain)
	d->symbols=NULL;
	d->parent=symTable;
	symTable=d;
	return d;
	}

void dropDomain(){
	Domain *d=symTable;
	symTable=d->parent;
	freeSymbols(d->symbols);
	free(d);
	}

void showNamedType(Type *t,const char *name){
	switch(t->tb){
		case TB_INT:printf("int");break;
		case TB_DOUBLE:printf("double");break;
		case TB_CHAR:printf("char");break;
		case TB_VOID:printf("void");break;
		default:		// TB_STRUCT
			printf("struct %s",t->s->name);
		}
	if(name)printf(" %s",name);
	if(t->n==0)printf("[]");
	else if(t->n>0)printf("[%d]",t->n);
	}

void showSymbol(Symbol *s){
	switch(s->kind){
			case SK_VAR:
				showNamedType(&s->type,s->name);
				printf(";\t// size=%d, idx=%d\n",typeSize(&s->type),s->varIdx);
				break;
			case SK_PARAM:{
				showNamedType(&s->type,s->name);
				printf(" /*size=%d, idx=%d*/",typeSize(&s->type),s->paramIdx);
				}break;
			case SK_FN:{
				showNamedType(&s->type,s->name);
				printf("(");
				bool next=false;
				for(Symbol *param=s->fn.params;param;param=param->next){
					if(next)printf(", ");
					showSymbol(param);
					next=true;
					}
				printf("){\n");
				for(Symbol *local=s->fn.locals;local;local=local->next){
					printf("\t");
					showSymbol(local);
					}
				printf("\t}\n");
				}break;
			case SK_STRUCT:{
				printf("struct %s{\n",s->name);
				for(Symbol *m=s->structMembers;m;m=m->next){
					printf("\t");
					showSymbol(m);
					}
				printf("\t};\t// size=%d\n",typeSize(&s->type));
				}break;
		}
	}

void showDomain(Domain *d,const char *name){
	printf("// domain: %s\n",name);
	for(Symbol *s=d->symbols;s;s=s->next){
		showSymbol(s);
		}
	puts("\n");
	}

Symbol *findSymbolInDomain(Domain *d,const char *name){
	for(Symbol *s=d->symbols;s;s=s->next){
		if(!strcmp(s->name,name))return s;
		}
	return NULL;
	}

Symbol *findSymbol(const char *name){
	for(Domain *d=symTable;d;d=d->parent){
		Symbol *s=findSymbolInDomain(d,name);
		if(s)return s;
		}
	return NULL;
	}

Symbol *addSymbolToDomain(Domain *d,Symbol *s){
	return addSymbolToList(&d->symbols,s);
	}

int allocInGlobalMemory(int nBytes){
	char *aux=(char*)realloc(globalMemory,nGlobalMemory+nBytes);
	if(!aux)err("not enough memory");
	int idx=nGlobalMemory;
	nGlobalMemory+=nBytes;
	return idx;
	}

Symbol *addExtFn(const char *name,void(*extFnPtr)(),Type ret){
	Symbol *fn=newSymbol(name,SK_FN);
	fn->fn.extFnPtr=extFnPtr;
	fn->type=ret;
	addSymbolToDomain(symTable,fn);
	return fn;
	}

Symbol *addFnParam(Symbol *fn,const char *name,Type type){
	Symbol *param=newSymbol(name,SK_PARAM);
	param->type=type;
	param->paramIdx=symbolsLen(fn->fn.params);
	addSymbolToList(&fn->fn.params,dupSymbol(param));
	return param;
	}

/*
-------------------Analiza de tipuri-----------------------
*/

typedef struct{
	Type type;		// tipul returnat
	bool lval;			// true daca este left-value
	bool ct;				// true daca este constanta
	}Ret;

bool canBeScalar(Ret* r){
	Type* t=&r->type;
	if(t->n>=0)return false;
	if(t->tb==TB_VOID)return false;
	return true;
	}

bool convTo(Type *src,Type *dst){
	// pointerii (arrays) se pot converti din unul in celalalt dar in nimic altceva
	if(src->n>=0){
		if(dst->n>=0)return true;
		return false;
		}
	if(dst->n>=0)return false;
	switch(src->tb){
		case TB_INT:
		case TB_DOUBLE:
		case TB_CHAR:
			switch(dst->tb){
				case TB_INT:
				case TB_CHAR:
				case TB_DOUBLE:
					return true;
				default:return false;
				}
		// o structura se poate converti doar la ea insasi
		case TB_STRUCT:
			if(dst->tb==TB_STRUCT&&src->s==dst->s)return true;
			return false;
		default:return false;
		}
	}

bool arithTypeTo(Type *t1,Type *t2,Type *dst){
	// nu exista operatii aritmetice cu pointeri
	if(t1->n>=0||t2->n>=0)return false;
	// rezultatul unei operatii aritmetice nu poate fi pointer sau structura
	dst->s=NULL;
	dst->n=-1;
	switch(t1->tb){
		case TB_INT:
			switch(t2->tb){
				case TB_INT:
				case TB_CHAR:
					dst->tb=TB_INT;return true;
				case TB_DOUBLE:dst->tb=TB_DOUBLE;return true;
				default:return false;
				}
		case TB_DOUBLE:
			switch(t2->tb){
				case TB_INT:
				case TB_DOUBLE:
				case TB_CHAR:
					dst->tb=TB_DOUBLE;return true;
				default:return false;
				}
		case TB_CHAR:
			switch(t2->tb){
				case TB_INT:
				case TB_DOUBLE:
				case TB_CHAR:
					dst->tb=t2->tb;return true;
				default:return false;
				}
		default:return false;
		}
	}
Symbol *findSymbolInList(Symbol *list,const char *name){
	for(Symbol *s=list;s;s=s->next){
			if(!strcmp(s->name,name))return s;
		}
	return NULL;
}    

//antete functii ASIN
bool unit();
bool structDef();
bool varDef();
bool typeBase(Type* t);
bool arrayDecl(Type* t);
bool fnDef();
bool fnParam();
bool stm();
bool stmCompound(bool newDomain);
bool expr();
bool exprAssign();
bool exprOr();
bool exprAnd();
bool exprEq();
bool exprRel();
bool exprAdd();
bool exprMul();
bool exprCast();
bool exprUnary();
bool exprPostfix();
bool exprPrimary();

//functia pentru consumarea atomilor lexicali
bool consume(int code){
    if(iTk->code == code){ // dacă la poziția curentă avem codul cerut, consumăm atomul
        consumedTk = iTk;
        iTk = iTk->next;
        return true;
    }
    return false; // dacă la poziția curentă se află un atom cu un alt cod decât cel cerut, nu are loc nicio acțiune
}

//exprPrimary: ID ( LPAR (expr( COMMA expr)* )? RPAR)?| CT_INT | CT_REAL | CT_CHAR | CT_STRING | LPAR expr RPAR

bool exprPrimary(Ret *r){
    Token *startTk = iTk;
    if(consume(ID)){
        Token* tkName = consumedTk;
        Symbol* s = findSymbol(tkName->text);
        if (!s) tkerr(iTk, "undefined id: %s", tkName->text);
        if(consume(LPAR)){
            if (s->kind != SK_FN) tkerr(iTk, "only a function can be called");
            Ret rArg;
            Symbol* param = s->fn.params;
            if(expr(&rArg)){
                if (!param) tkerr(iTk, "too many arguments in function call");
                if (!convTo(&rArg.type, &param->type)) tkerr(iTk, "in call, cannot convert the argument type to the parameter type");
                param = param->next;
                for(;;){
                    if(consume(COMMA)){
                        if(expr(&rArg)){
                            if (!param) tkerr(iTk, "too many arguments in function call");
                            if (!convTo(&rArg.type, &param->type)) tkerr(iTk, "in call, cannot convert the argument type to the parameter type");
                            param = param->next;
                        }
                        else {
                            tkerr(iTk, "lipseste expresia dupa ,");
                        }
                    }
                    else break;
                }
            }
            if(consume(RPAR)){
                if (param) tkerr(iTk, "too few arguments in function call");
                *r = (Ret){ s->type,false,true };
                return true;
            } else tkerr(iTk, "lipseste ) din expresia (...)");
        }
        if (s->kind == SK_FN) tkerr(iTk, "a function can only be called");
        *r = (Ret){ s->type,true,s->type.n >= 0 };
        return true;
    }
    if(consume(CT_INT)){
        *r = (Ret){ {TB_INT,NULL,-1},false,true };
        return true;
    }
    if(consume(CT_REAL)){
        *r = (Ret){ {TB_DOUBLE,NULL,-1},false,true };
        return true;
    }
    if(consume(CT_CHAR)){
        *r = (Ret){ {TB_CHAR,NULL,-1},false,true };
        return true;
    }
    if(consume(CT_STRING)){
        *r = (Ret){ {TB_CHAR,NULL,0},false,true };
        return true;
    }
    if(consume(LPAR)){
        if(expr(r)){
            if(consume(RPAR)){
                return true;
            } else tkerr(iTk, "lipseste ) din (...)");
        } else tkerr(iTk, "expresie invalida dupa (");
    }
    iTk = startTk;
    return false;
}

//exprPostfix:exprPostfix LBRACKET expr RBRACKET | exprPostfix DOT ID | exprPrimary

bool exprPostfixPrim(Ret *r){
    Token *start = iTk;
    if(consume(LBRACKET)){
        Ret idx;
        if(expr(&idx)){
            if(consume(RBRACKET)){
                if (r->type.n < 0) tkerr(iTk, "only an array can be indexed");
                Type tInt = { TB_INT,NULL,-1 };
                if (!convTo(&idx.type, &tInt)) tkerr(iTk, "the index is not convertible to int");
                r->type.n = -1;
                r->lval = true;
                r->ct = false;
                if(exprPostfixPrim(r)){
                    return true;
                }
            } else tkerr(iTk, "lipseste ] dupa [...");
        } else tkerr(iTk, "expresie invalida dupa [");
    }

    if(consume(DOT)){
        if(consume(ID)){
            Token* tkName = consumedTk;
            if (r->type.tb != TB_STRUCT) tkerr(iTk, "a field can only be selected from a struct");
            Symbol* s = findSymbolInList(r->type.s->structMembers, tkName->text);
            if (!s) tkerr(iTk, "the structure %s does not have a field %s",r->type.s->name,tkName->text);
            * r = (Ret){ s->type,true,s->type.n >= 0 };
            if(exprPostfixPrim(r)){
                return true;
            }
        } else tkerr(iTk, "expresie invalida dupa .");
    }
    iTk = start;
    return true;
}

bool exprPostfix(Ret *r){
    Token *start=iTk;
    if(exprPrimary(r)){
        if(exprPostfixPrim(r)){
            return true;
        }
    }
    iTk = start;
    return false;
}

//exprUnary: ( SUB | NOT )exprUnary|exprPostfix

bool exprUnary(Ret *r){
    Token *start=iTk;
    char aux;
    if(iTk->code == SUB) aux = '-';
    if(iTk->code == NOT) aux = '!';
    if(consume(SUB) || consume(NOT)){
        if(exprUnary(r)){
            if (!canBeScalar(r)) tkerr(iTk, "unary - must have a scalar operand");
            r->lval = false;
            r->ct = true;
            return true;
        } else tkerr(iTk, "expresie invalida dupa %c", aux);
    }
    if(exprPostfix(r)){
        return true;
    }
    iTk = start;
    return false;
}

//exprCast: LPAR typeBase arrayDecl? RPAR exprCast | exprUnary

bool exprCast(Ret *r){
    Token *start = iTk;
    //Type t;
    if(consume(LPAR)){
        Type t;
        Ret op;
        if(typeBase(&t)){
            if(arrayDecl(&t)){
            }
            if(consume(RPAR)){
                if(exprCast(&op)){
                    if (t.tb == TB_STRUCT) tkerr(iTk, "cannot convert to a struct type");
                    if (op.type.tb == TB_STRUCT) tkerr(iTk, "cannot convert a struct");
                    if (op.type.n >= 0 && t.n < 0) tkerr(iTk, "an array can be converted only to another array");
                    if (op.type.n < 0 && t.n >= 0) tkerr(iTk, "a scalar can be converted only to another scalar");
                    *r = (Ret){ t,false,true };
                    return true;
                } else tkerr(iTk, "expresie invalida dupa )");
            } else tkerr(iTk, "lipseste ) din (...)");
        } else tkerr(iTk, "lipseste tipul dupa (");
    }

    if(exprUnary(r)){
        return true;
    }

    iTk = start;
    return false;
}

//exprMul:exprMul( MUL | DIV )exprCast|exprCast

bool exprMulPrim(Ret *r){
    char aux;
    if(iTk->code == MUL) aux = '*';
    if(iTk->code == DIV) aux = '/';
    if(consume(MUL) || consume(DIV)){
        Ret right;
        if(exprCast(&right)){
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(iTk, "inavlid operand type for *");
            *r = (Ret){ tDst, false, true };
            if(exprMulPrim(r)){
                return true;
            }
        } else tkerr(iTk, "expresie invalida dupa %c", aux);
    }
    return true;
}

bool exprMul(Ret *r){
    Token* start = iTk;
    if(exprCast(r)){
        if(exprMulPrim(r)){
            return true;
        }
    }
    iTk=start;
    return false;
}

//exprAdd:exprAdd( ADD | SUB )exprMul|exprMul

bool exprAddPrim(Ret *r){
    char aux;
    if(iTk->code == ADD) aux = '+';
    if(iTk->code == SUB) aux = '-';
    if(consume(ADD) || consume(SUB)){
        Ret right;
        if(exprMul(&right)){
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(iTk, "inavlid operand type for +");
            *r = (Ret){ tDst, false, true };
            if(exprAddPrim(r)){
                return true;
            }
        } else tkerr(iTk, "expresie invalida dupa %c", aux);
    }
    return true;
}

bool exprAdd(Ret *r){
    Token *start=iTk;
    if(exprMul(r)){
        if(exprAddPrim(r)){
            return true;
        }
    }

    iTk=start;
    return false;
}

//exprRel:exprRel( LESS | LESSEQ | GREATER | GREATEREQ)exprAdd|exprAdd

bool exprRelPrim(Ret *r){
    char aux[3];
    if(iTk->code == LESS) strcpy(aux, "<");
    if(iTk->code == LESSEQ) strcpy(aux, "<=");
    if(iTk->code == GREATER) strcpy(aux, ">");
    if(iTk->code == GREATEREQ) strcpy(aux, ">=");
    if(consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)){
        Ret right;
        if(exprAdd(&right)){
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(iTk, "inavlid operand type for <\n");
            *r = (Ret){ {TB_INT, NULL, -1}, false, true };
            if(exprRelPrim(r)){
                return true;
            }
        } else tkerr(iTk, "expresie invalida dupa %s", aux);
    }
    return true;
}

bool exprRel(Ret *r){
    Token *start = iTk;
    if(exprAdd(r)){
        if(exprRelPrim(r)){
            return true;
        }
    }
    iTk=start;
    return false;
}

//exprEq:exprEq( EQUAL | NOTEQ )exprRel|exprRel

bool exprEqPrim(Ret *r){
    char aux[3];
    if(iTk->code == EQUAL) strcpy(aux, "=");
    if(iTk->code == NOTEQ) strcpy(aux, "!=");
    if(consume(EQUAL) || consume(NOTEQ)){
        Ret right;
        if(exprRel(&right)){
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(iTk, "inavlid operand type for ==");
            *r = (Ret){ {TB_INT, NULL, -1}, false, true };
            if(exprEqPrim(r)){
                return true;
            }
        } else tkerr(iTk, "expresie invalida dupa %s", aux);
    }
    return true;
}

bool exprEq(Ret *r){
    Token *start=iTk;
    if(exprRel(r)){
        if(exprEqPrim(r)){
            return true;
        }
    }
    iTk=start;
    return false;
}

//exprAnd:exprAnd AND exprEq | exprEq

bool exprAndPrim(Ret *r){
    Token* start = iTk;
    if(consume(AND)){
        Ret right;
        if(exprEq(&right)){
            Type tDst;
            if(!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(iTk, "inavlid operand type for &&");
            *r = (Ret){ {TB_INT, NULL, -1}, false, true };
            if(exprAndPrim(r)){
                return true;
            }
        } else tkerr(iTk, "expresie invalida dupa &&");
    }
    return true;
}

bool exprAnd(Ret *r){
    Token *start=iTk;
    if(exprEq(r)){
        if(exprAndPrim(r)){
            return true;
        }
    } 
    iTk=start;
    return false;
}

//exprOr:exprOr OR exprAnd | exprAnd

bool exprOrPrim(Ret *r){
    if(consume(OR)){
        Ret right;
        if(exprAnd(&right)){
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(iTk, "inavlid operand type for ||");
            *r = (Ret){ {TB_INT, NULL, -1}, false, true };
            if(exprOrPrim(r)){
                return true;
            } else tkerr(iTk, "Expresie invalida dupa &&");
        } else tkerr(iTk, "expresie invalida dupa ||"); 
    }
    return true;
}

bool exprOr(Ret *r){
    Token* start = iTk;
    if(exprAnd(r)){
        if(exprOrPrim(r)){
            return true;
        }
    }
    iTk=start;
    return false;
}

//exprAssign:exprUnary ASSIGN exprAssign | exprOr

bool exprAssign(Ret *r){
    Token* start = iTk;
    Ret rDst;
    if(exprUnary(&rDst)){
        if(consume(ASSIGN)){
            if(exprAssign(r)){
                if (!rDst.lval) tkerr(iTk, "the assign destination must be a left-value\n");
                if (rDst.ct) tkerr(iTk, "the assign destination cannot be constant");
                if (!canBeScalar(&rDst)) tkerr(iTk, "the assign destination must be scalar");
                if (!canBeScalar(r)) tkerr(iTk, "the assign source must be scalar");
                if (!convTo(&r->type, &rDst.type)) tkerr(iTk, "the assign source cannot be converted to destination");
                r->lval = false;
                r->ct = true;
                return true;
            } else tkerr(iTk, "lipseste expresie dupa =");
        }// else tkerr(iTk, "lipseste = din declarare");
        iTk = start;
    }
    
    if(exprOr(r)){
        return true;
    }
    iTk=start;
    return false;
}

//expr:exprAssign

bool expr(Ret *r){
    if(exprAssign(r)){
        return true;
    }
    return false;
}

//stmCompound: LACC (varDef|stm)* RACC

bool stmCompound(bool newDomain){
    Token *start=iTk;
    if(consume(LACC)){
        if(newDomain) pushDomain();
        for(;;){
            if(varDef()){}
            else if(stm()){}
            else break;
        }
        if(consume(RACC)){
            if (newDomain) dropDomain();
            return true;
        } else tkerr(iTk, "lipseste } dupa { ...");
    }
    iTk=start;
    return false;
}

//stm:stmCompound| IF LPAR expr RPAR stm ( ELSE stm)?| WHILE LPARexprRPARstm| FOR LPARexpr? SEMICOLONexpr? SEMICOLONexpr? RPARstm| BREAK SEMICOLON| RETURNexpr? SEMICOLON|expr? SEMICOLON

bool stm(){
    Token *start=iTk;
    Ret rInit, rCond, rStep, rExpr;
    if(stmCompound(true)){
        return true;
    }
    if(consume(IF)){
        if(consume(LPAR)){
            if(expr(&rCond)){
                if(!canBeScalar(&rCond)) tkerr(iTk, "the if condition must be a scalar value");
                if(consume(RPAR)){
                    if(stm()){
                        if(consume(ELSE)){
                            if(stm()){
                            } else tkerr(iTk, "lipseste corpul else-ului");
                        }
                        return true;
                    } else tkerr(iTk, "lipseste corpul if-ului");
                } else tkerr(iTk, "lipseste ) dupa expresia din if");
            } else tkerr(iTk, "lipseste expresia din if");
        } else tkerr(iTk, "lipseste ( dupa if");
    }
    if(consume(WHILE)){
        if(consume(LPAR)){
            if(expr(&rCond)){
                if(consume(RPAR)){
                    if(!canBeScalar(&rCond)) tkerr(iTk,"the while condition must be a scalar value");
                    if(stm()){
                        return true;
                    } else tkerr(iTk, "lipseste corpul while-ului");
                } else tkerr(iTk, "lipseste ) dupa conditia din while");
            } else tkerr(iTk, "lipseste expresia din while");
        } else tkerr(iTk, "lipseste ( dupa while");
    }
    if(consume(FOR)){
        if(consume(LPAR)){
            if(expr(&rInit)){}
            if(consume(SEMICOLON)){
                if(expr(&rCond)){
                    if (!canBeScalar(&rCond)) tkerr(iTk, "the for condition must be a scalar value\n");
                }
                if(consume(SEMICOLON)){
                    if(expr(&rStep)){}
                    if(consume(RPAR)){
                        if(stm()){
                            return true;
                        } else tkerr(iTk, "lipseste corpul for-ului");
                    } else tkerr(iTk, "lipseste ) din for");
                } else tkerr(iTk, "lipseste al doilea ; din for");
            } else tkerr(iTk, "lipseste primul ; din for");
        } else tkerr(iTk, "lipseste ( dupa for");
    }
    if(consume(BREAK)){
        if(consume(SEMICOLON)){
            return true;
        } else tkerr(iTk, "lipseste ; dupa break");
    }
    if(consume(RETURN)){
        if(expr(&rExpr)){
            if (owner->type.tb == TB_VOID) tkerr(iTk, "a void function cannot return value");
            if (!canBeScalar(&rExpr)) tkerr(iTk, "the return value must be a scalar value");
            if (!convTo(&rExpr.type, &owner->type)) tkerr(iTk, "cannot convert the return expression type to the function return type");
        }
        else if (owner->type.tb != TB_VOID) tkerr(iTk, "a non-void function must return a value\n");
        
        if(consume(SEMICOLON)){
            return true;
        } else tkerr(iTk, "lipseste ; dupa return");
    }
    if(expr(&rExpr)){
        if(consume(SEMICOLON)){
        return true;
        } else tkerr(iTk, "lipseste ;");
    }

    if(consume(SEMICOLON)){
        return true;
    }
    
    iTk = start;
    return false;
}

//fnParam:typeBase ID arrayDecl?
bool fnParam() {
    Token *start=iTk;
    Type t;
    if(typeBase(&t)){
        if(consume(ID)){
            Token *tkName = consumedTk;
            if(arrayDecl(&t)){}
            Symbol* param = findSymbolInDomain(symTable, tkName->text);
            if (param) tkerr(iTk, "symbol redefinition: %s\n", tkName->text);
            param = newSymbol(tkName->text, SK_PARAM);
            param->type = t;
            param->paramIdx = symbolsLen(owner->fn.params);
            addSymbolToDomain(symTable, param);
            addSymbolToList(&owner->fn.params, dupSymbol(param));
            return true;
        } else tkerr(iTk, "lipseste numele parametrului");
    }
    iTk = start;
    return false;
}

//fnDef: (typeBase | VOID ) ID LPAR (fnParam ( COMMA fnParam)* )? RPARstmCompound
bool fnDef(){
    Token *start=iTk;
    Type t;
    if(typeBase(&t)){
        if(consume(ID)){
            Token *tkName = consumedTk;
            if(consume(LPAR)){
                Symbol *fn=findSymbolInDomain(symTable,tkName->text);
                if(fn) tkerr(iTk,"symbol redefinition: %s",tkName->text);
                fn=newSymbol(tkName->text,SK_FN);
                fn->type = t;
                addSymbolToDomain(symTable,fn);
                owner=fn;
                pushDomain();
                if(fnParam()){
                    for(;;){
                        if(consume(COMMA)){
                            if(fnParam()){
                            }
                            else tkerr(iTk, "Lipseste din definitia functiei argumentul de dupa ,");
                        }
                        else break;
                    }
                }
                if(consume(RPAR)){
                    if(stmCompound(false)){
                        dropDomain();
                        owner=NULL;
                        return true;
                    } else tkerr(iTk, "lipseste corpul functiei");
                } else tkerr(iTk, "lipseste ) din declararea functia");
            }
        } else tkerr(iTk, "lipseste numele functiei declarate");
    }

    if(consume(VOID)){
        t.tb=TB_VOID;
        if(consume(ID)){
            Token *tkName = consumedTk;
            if(consume(LPAR)){
                Symbol *fn=findSymbolInDomain(symTable,tkName->text);
                if(fn) tkerr(iTk,"symbol redefinition: %s",tkName->text);
                fn=newSymbol(tkName->text,SK_FN);
                fn->type = t;
                addSymbolToDomain(symTable,fn);
                owner=fn;
                pushDomain();
                if(fnParam()){
                    for(;;){
                        if(consume(COMMA)){
                            if(fnParam()){
                            }
                            else tkerr(iTk, "Lipseste din definitia functiei argumentul de dupa ,");
                        }
                        else break;
                    }
                }
                if(consume(RPAR)){
                    if(stmCompound(false)){
                        dropDomain();
                        owner=NULL;
                        return true;
                    } else tkerr(iTk, "lipseste corpul functiei");
                } else tkerr(iTk, "lipseste ) din declararea functia");
            } else tkerr(iTk, "lipseste ( din declaratia functiei");
        } else tkerr(iTk, "lipseste numele functii declarate");
    }
    iTk = start;
    return false;
}

//arrayDecl: LBRACKET CT_INT? RBRACKET
bool arrayDecl(Type *t){
    Token *start = iTk;
    if(consume(LBRACKET)){
        if(consume(CT_INT)){
            Token *tkSize = consumedTk;
            t->n = tkSize->i;
        }
        else t->n = 0;
        if(consume(RBRACKET)){
            return true;
        } else tkerr(iTk, "Lipseste ] din declararea array-ului");
    }
    iTk = start;
    return false;
}

//typeBase: INT | DOUBLE | CHAR | STRUCT ID
bool typeBase(Type *t) {
    Token *start = iTk;
    t->n = -1;
    if(consume(INT)){
        t->tb = TB_INT;
        return true;
    }
    if(consume(DOUBLE)){
        t->tb = TB_DOUBLE;
        return true;
    }
    if(consume(CHAR)){
        t->tb = TB_CHAR;
        return true;
    }
    if(consume(STRUCT)){
        if(consume(ID)){
            Token *tkName = consumedTk;
            t->tb = TB_STRUCT;
            t->s = findSymbol(tkName->text);
            if(!t->s)tkerr(iTk,"structura nedefinita: %s",tkName->text);
            return true;
        } else tkerr(iTk, "lipseste numele structurii in declararea tipului");
    }
    iTk = start;
    return false;
}

//varDef:typeBase ID arrayDecl? SEMICOLON
bool varDef(){
    Token *start = iTk;
    Type t;
    if(typeBase(&t)){
        if(consume(ID)){
            Token *tkName = consumedTk;
            if(arrayDecl(&t)){
                if(t.n == 0) tkerr(iTk, "O variabila vector ar trebui sa aiba declarata dimensiunea");
            }
            if(consume(SEMICOLON)){
                Symbol *var=findSymbolInDomain(symTable,tkName->text);
                if(var) tkerr(iTk,"symbol redefinition: %s",tkName->text);
                var=newSymbol(tkName->text,SK_VAR);
                var->type=t;
                var->owner = owner;
                addSymbolToDomain(symTable,var);
                if (owner) {
                    switch (owner->kind) {
                    case SK_FN:
                        var->varIdx = symbolsLen(owner->fn.locals);
                        addSymbolToList(&owner->fn.locals, dupSymbol(var));
                        break;
                    case SK_STRUCT:
                        var->varIdx = typeSize(&owner->type);
                        addSymbolToList(&owner->structMembers, dupSymbol(var));
                        break;
                    }
                }
                else {
                    var->varIdx = allocInGlobalMemory(typeSize(&t));
                }
                return true;
            } else tkerr(iTk, "lipseste ; de la finalul declararii variabilei");
        } else tkerr(iTk, "lipseste numele variabilei declarate");
    }
    iTk = start;
    return false;
}

//structDef: STRUCT ID LACC varDef* RACC SEMICOLON
bool structDef(){
    Token* start = iTk;
    if(consume(STRUCT)){
        if(consume(ID)){
            Token* tkName = consumedTk;
            if(consume(LACC)){
                Symbol *s=findSymbolInDomain(symTable,tkName->text);
                if(s) tkerr(iTk,"symbol redefinition: %s",tkName->text);
                s=addSymbolToDomain(symTable,newSymbol(tkName->text,SK_STRUCT));
                s->type.tb=TB_STRUCT;
                s->type.s=s;
                s->type.n=-1;
                pushDomain();
                owner=s;
                for(;;){
                    if(varDef()){}
                    else break;
                }
                if(consume(RACC)){
                    if(consume(SEMICOLON)){
                        owner=NULL;
                        dropDomain();
                        return true;
                    } else tkerr(iTk, "lipseste ; din finalul declararii structurii");
                } else tkerr(iTk, "lipseste } pt a incheia corpul structurii");
            }
        } else tkerr(iTk, "lipseste numele structurii");
    }
    iTk = start;
    return false;
}

//unit: (structDef | fnDef | varDef)* END
bool unit(){
    for(;;){
        if(structDef() || fnDef() || varDef()){
        }
        else break;
    }
    if(consume(END)){
        return true;
    }
    return false;
}

//analizorul Sintactic functia mare
void Asin(){
    if(unit()){
    }
}

/*
--------Masina virtuala pe stiva-----------
*/
// masina virtuala stiva

// instructiunile masinii virtuale
// FORMAT: OP_<name>.<data_type>    // [argument] effect
//		OP_ - prefix comun (operation code)
//		<name> - numele instructiunii
//		<data_type> - daca este prezent, tipul de date asupra caruia actioneaza instructiunea
//			.i - int
//			.f - double
//			.c - char
//			.p - pointer
//		[argument] - daca este prezent, argumentul instructiunii
//		effect - efectul instructiunii
enum{
	OP_HALT	// incheie executia codului 
	,OP_PUSH_I		// [ct.i] depune pe stiva constanta ct.i
	,OP_CALL			// [idx] apeleaza o functie care se afla la indexul idx in vectorul de instructiuni
	,OP_CALL_EXT	// [addr] apeleaza o functie din interiorul compilatorului, de la adresa addr
	,OP_ENTER		// [nb_locals] creaza cadrul unei functii cu nb variabile locale
	,OP_RET				// [nb_params] revine dintr-o functie care are nb parametri si returneaza o valoare
	,OP_RET_VOID	// [nb_params] revine dintr-o functie care are nb parametri, fara sa returneze o valoare
	,OP_CONV_I_F	// converteste valoarea de pe stiva de la int la double
	,OP_JMP				// salt neconditionat la indexul specificat
	,OP_JF				// salt la indexul specificat daca pe stiva este false
	,OP_JT				// salt la indexul specificat daca pe stiva este true
	,OP_FPLOAD		// [idx] depune pe stiva valoarea de la FP[idx]
	,OP_FPSTORE		// [idx] stocheaza la FP[idx] valoarea din varful stivei
	,OP_ADD_I			// aduna 2 valori intregi din varful stivei si depune rezultatul in stiva
	,OP_LESS_I			// compara 2 valori intregi din varful stivei si depune rezultatul in stiva ca int

	// instructiuni adaugate pentru generarea de cod
	,OP_PUSH_F		// [ct.i] depune pe stiva constanta ct.i
	,OP_CONV_F_I	// converteste valoarea de pe stiva de la double la int
	,OP_LOAD_I		// preia o adresa din stiva si depune in locul ei valoarea intreaga de la aceasta adresa
	,OP_LOAD_F		// preia o adresa din stiva si depune in locul ei valoarea reala de la aceasta adresa
	,OP_STORE_I		// preia din stiva o adresa si o valoare intreaga si depune valoarea la adresa specificata. Lasa pe stiva valoarea.
	,OP_STORE_F		// preia din stiva o adresa si o valoare reala si depune valoarea la adresa specificata. Lasa pe stiva valoarea.
	,OP_ADDR			// [idx] depune pe stiva adresa lui globalMemory[idx]
	,OP_FPADDR_I		// [idx] depune pe stiva adresa lui FP[idx].i
	,OP_FPADDR_F		// [idx] depune pe stiva adresa lui FP[idx].f
	,OP_ADD_F				// aduna 2 valori reale din varful stivei si depune rezultatul in stiva
	,OP_SUB_I				// scade 2 valori intregi din varful stivei si depune rezultatul in stiva
	,OP_SUB_F				// scade 2 valori reale din varful stivei si depune rezultatul in stiva
	,OP_MUL_I				// inmulteste 2 valori intregi din varful stivei si depune rezultatul in stiva
	,OP_MUL_F				// inmulteste 2 valori reale din varful stivei si depune rezultatul in stiva
	,OP_DIV_I				// imparte 2 valori intregi din varful stivei si depune rezultatul in stiva
	,OP_DIV_F				// imparte 2 valori reale din varful stivei si depune rezultatul in stiva
	,OP_LESS_F			// compara 2 valori reale din varful stivei si depune rezultatul in stiva ca int
	,OP_DROP				// sterge valoarea din varful stivei
	};

// o valoare de pe stiva sau argumentul unei instructiuni
typedef union{
	int i;			// valori intregi si indecsi
	double f;		// valori float
	void *p;		// pointeri
	void(*extFnPtr)();		// pointer la o functie externa
	}Val;

// o instructiune a masinii virtuale
typedef struct{
	int op;		// opcode: OP_*
	Val arg;
	}Instr;

Instr *instructions=NULL;
int nInstructions=0;

int addInstr(int op){
	Instr *p=(Instr*)realloc(instructions,(nInstructions+1)*sizeof(Instr));
	if(!p)err("not enough memory");
	instructions=p;
	Instr *i=instructions+nInstructions;
	i->op=op;
	return nInstructions++;
	}

int insertInstr(int pos,int op){
	Instr *p=(Instr*)realloc(instructions,(nInstructions+1)*sizeof(Instr));
	if(!p)err("not enough memory");
	instructions=p;
	Instr *i=instructions+pos;
	memmove(i+1,i,(nInstructions-pos)*sizeof(Instr));
	i->op=op;
	nInstructions++;
	return pos;
	}

int addInstrWithInt(int op,int i){
	int pos=addInstr(op);
	instructions[pos].arg.i=i;
	return pos;
	}

int addInstrWithDouble(int op,double d){
	int pos=addInstr(op);
	instructions[pos].arg.f=d;
	return pos;
	}

Val stack[10000];		// stiva
Val *SP;		// Stack pointer - varful stivei - indica intotdeauna valoarea din varful stivei

void pushv(Val v){
	if(SP+1==stack+10000)err("trying to push into a full stack");
	*++SP=v;
	}

Val popv(){
	if(SP==stack-1)err("trying to pop from empty stack");
	return *SP--;
	}

void pushi(int i){
	if(SP+1==stack+10000)err("trying to push into a full stack");
	(++SP)->i=i;
	}

double popf(){
	if(SP==stack-1)err("trying to pop from empty stack");
	return SP--->f;
	}

void pushf(double f){
	if(SP+1==stack+10000)err("trying to push into a full stack");
	(++SP)->f=f;
	}

int popi(){
	if(SP==stack-1)err("trying to pop from empty stack");
	return SP--->i;
	}

void pushp(void *p){
	if(SP+1==stack+10000)err("trying to push into a full stack");
	(++SP)->p=p;
	}

void *popp(){
	if(SP==stack-1)err("trying to pop from empty stack");
	return SP--->p;
	}

void put_i(){
	printf("=> %d",popi());
	}

void put_d(){
	printf("=> %f",popf());
	}

void mvInit(){
	Symbol *fn=addExtFn("put_i",put_i,(Type){TB_VOID,NULL,-1});
	addFnParam(fn,"i",(Type){TB_INT,NULL,-1});
    Symbol *fn2=addExtFn("put_d",put_d,(Type){TB_VOID,NULL,-1});
	addFnParam(fn2,"i",(Type){TB_INT,NULL,-1});
	}

void run(){
	SP=stack-1;
	Val *FP=NULL;		// valoarea initiala nu conteaza
	Instr *IP=instructions;		// Instruction pointer - pointer la instructiunea curenta
	Val v;
	double iTop,iBefore;
    int iArg;
	double fTop;
	void *pTop;
	void(*extFnPtr)();
	for(;;){
		// afiseaza indexul instructiunii curente
		// si numarul de valori din stiva
		printf("%03d/%d\t",(int)(IP-instructions),(int)(SP-stack+1));
		switch(IP->op){
			case OP_HALT:
				printf("HALT");
				return;
			case OP_PUSH_I:
				printf("PUSH.i\t%d",IP->arg.i);
				pushi(IP->arg.i);
				IP++;
				break;
			case OP_CALL:
				pushp(IP+1);
				printf("CALL\t%d",IP->arg.i);
				IP=instructions+IP->arg.i;
				break;
			case OP_CALL_EXT:
				extFnPtr=IP->arg.extFnPtr;
				printf("CALL_EXT\t%p\n",extFnPtr);
				extFnPtr();
				IP++;
				break;
			case OP_ENTER:
				pushp(FP);
				FP=SP;
				SP+=IP->arg.i;
				printf("ENTER\t%d",IP->arg.i);
				IP++;
				break;
			case OP_RET_VOID:
				iArg=IP->arg.i;
				printf("RET_VOID\t%d",iArg);
				IP=FP[-1].p;
				SP=FP-iArg-2;
				FP=FP[0].p;
				break;
			case OP_JMP:
				printf("JMP\t%d",IP->arg.i);
				IP=instructions+IP->arg.i;
				break;
			case OP_JF:
				iTop=popf();
				printf("JF\t%d\t// %d",IP->arg.i,iTop);
				IP=iTop ? IP+1 : instructions+IP->arg.i;
				break;
			case OP_FPLOAD:
				v=FP[IP->arg.i];
				pushv(v);
				printf("FPLOAD\t%d\t// i:%d, f:%g",IP->arg.i,v.i,v.f);
				IP++;
				break;
			case OP_FPSTORE:
				v=popv();
				FP[IP->arg.i]=v;
				printf("FPSTORE\t%d\t// i:%d, f:%g",IP->arg.i,v.i,v.f);
				IP++;
				break;
			case OP_ADD_I:
				iTop=popi();
				iBefore=popi();
				pushi(iBefore+iTop);
				printf("ADD.i\t// %d+%d -> %d",iBefore,iTop,iBefore+iTop);
				IP++;
				break;
            case OP_ADD_F:
				iTop=popf();
				iBefore=popf();
				pushf(iBefore+iTop);
				printf("ADD.f\t// %f+%f -> %f",iBefore,iTop,iBefore+iTop);
				IP++;
				break;
			case OP_LESS_I:
				iTop=popi();
				iBefore=popi();
				pushi(iBefore<iTop);
				printf("LESS.i\t// %d<%d -> %d",iBefore,iTop,iBefore<iTop);
				IP++;
				break;
            case OP_LESS_F:
				iTop=popf();
				iBefore=popf();
				pushf(iBefore<iTop);
				printf("LESS.f\t// %f<%f -> %d",iBefore,iTop,iBefore<iTop);
				IP++;
				break;
			// instructiuni adaugate pentru generarea de cod
			case OP_RET:
				v=popv();
				iArg=IP->arg.i;
				printf("RET\t%d\t// i:%d, f:%g",iArg,v.i,v.f);
				IP=FP[-1].p;
				SP=FP-iArg-2;
				FP=FP[0].p;
				pushv(v);
				break;
			case OP_PUSH_F:
				printf("PUSH.f\t%g",IP->arg.f);
				pushf(IP->arg.f);
				IP++;
				break;
			case OP_CONV_F_I:
				fTop=popf();
				pushi((int)fTop);
				printf("CONV.f.i\t// %g -> %d",fTop,(int)fTop);
				IP++;
				break;
			case OP_LOAD_I:
				pTop=popp();
				pushi(*(int*)pTop);
				printf("LOAD.i\t// *(int*)%p -> %d",pTop,*(int*)pTop);
				IP++;
				break;
			case OP_STORE_I:
				iTop=popi();
				v=popv();
				*(int*)v.p=iTop;
				pushi(iTop);
				printf("STORE.i\t//*(int*)%p=%d",v.p,iTop);
				IP++;
				break;
			case OP_FPADDR_I:
				pTop=&FP[IP->arg.i].i;
				pushp(pTop);
				printf("FPADDR\t%d\t// %p",IP->arg.i,pTop);
				IP++;
				break;
			case OP_SUB_I:
				iTop=popi();
				iBefore=popi();
				pushi(iBefore-iTop);
				printf("SUB.i\t// %d-%d -> %d",iBefore,iTop,iBefore-iTop);
				IP++;
				break;
			case OP_MUL_I:
				iTop=popi();
				iBefore=popi();
				pushi(iBefore*iTop);
				printf("MUL.i\t// %d*%d -> %d",iBefore,iTop,iBefore*iTop);
				IP++;
				break;
			case OP_DROP:
				popv();
				printf("DROP");
				IP++;
				break;
			default:err("run: instructiune neimplementata: %d",IP->op);
			}
		putchar('\n');
		}
	}

/* Programul codifica urmatorul cod sursa:
f(2);
void f(int n){		// cadru: n[-2] ret[-1] oldFP[0] i[1]
	int i;
	for(i=0;i<n;i=i+1){
		put_i(i);
		}
	}
*/
void genTestProgram(){
	addInstrWithDouble(OP_PUSH_F,2.0);
	int callPos=addInstr(OP_CALL);
	addInstr(OP_HALT);
	instructions[callPos].arg.i=nInstructions;
	addInstrWithInt(OP_ENTER,1);
	// i=0;  (componentele lui for sunt implementate sub forma unui while)
	addInstrWithDouble(OP_PUSH_F,0.0);
	addInstrWithInt(OP_FPSTORE,1);
	// while(i<n){
	int whilePos=addInstrWithInt(OP_FPLOAD,1);
	addInstrWithInt(OP_FPLOAD,-2);
	addInstr(OP_LESS_F);
	int jfAfterFor=addInstr(OP_JF);
	// put_d(d);
	addInstrWithInt(OP_FPLOAD,1);
	Symbol *s=findSymbol("put_d");
	if(!s)err("undefined: put_d");
	int putiCall=addInstr(OP_CALL_EXT);
	instructions[putiCall].arg.extFnPtr=s->fn.extFnPtr;
	// i=i+1
	addInstrWithInt(OP_FPLOAD,1);
	addInstrWithDouble(OP_PUSH_F,0.5);
	addInstr(OP_ADD_F);
	addInstrWithInt(OP_FPSTORE,1);
	// } ( urmatoarea iteratie )
	addInstrWithInt(OP_JMP,whilePos);
	// revenire din functie
	instructions[jfAfterFor].arg.i=nInstructions;
	addInstrWithInt(OP_RET_VOID,1);
	}

/*
--------generearea de cod------------
*/
void insertConvIfNeeded(int pos,Type *srcType,Type *dstType){
	switch(srcType->tb){
		case TB_INT:
			switch(dstType->tb){
				case TB_DOUBLE:
					insertInstr(pos,OP_CONV_I_F);
					break;
				}
			break;
		case TB_DOUBLE:
			switch(dstType->tb){
				case TB_INT:
					insertInstr(pos,OP_CONV_F_I);
					break;
				}
			break;
		}
	}

void addRVal(bool lval,Type *type){
	if(!lval)return;
	switch(type->tb){
		case TB_INT:
			addInstr(OP_LOAD_I);
			break;
		case TB_DOUBLE:
			addInstr(OP_LOAD_F);
			break;
		}
	}


int main(){
    //fisier de intrare
    FILE *fin;
    fin = fopen("testalex.c", "r");
    if(!fin){
        printf("Eroare la deschiderea fisierului!\n");
        exit(2);
    }

    //citirea din fisier in buffer-ul de intrare
    char c;
    int i = 0;
    while((c = fgetc(fin)) != EOF){
        inBuff[i] = c;
        i++;
    }
    inBuff[i] = '\0'; //setam ca \0 ultimul element din fisier
    fclose(fin);
    
    pCrtCh = inBuff; //setam caracterul de la care incepe, ca fiind inceputul sirului
    while(getNextToken() != END){
    }
    showAtoms();
    printf("\n");
    pushDomain();
    iTk = tokens; //atribuirea lui iTk cu pointer-ul primului atom lexical
    //Asin();
    mvInit(); //initializare masina virtuala
    showDomain(symTable, "global");
    
    genTestProgram(); //genereaza cod de test pentru masina virtuala
    run(); //executie cod masina virtuala
    dropDomain();
    printf("\n");
    return 0;
}