#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

enum{ID, BREAK, CHAR, DOUBLE, ELSE, FOR, IF, INT, RETURN, STRUCT, VOID, WHILE, CT_INT ,CT_REAL, CT_CHAR, CT_STRING,
COMMA, SEMICOLON, LPAR, RPAR, LBRACKET, RBRACKET, LACC, RACC, END, ADD, SUB, MUL, DIV, DOT, AND, OR, NOT, ASSIGN,
EQUAL, NOTEQ, LESS, LESSEQ, GREATER, GREATEREQ}; // codurile AL

typedef struct _Token{
    int code; // codul (numele)
    union{
        char *text; // folosit pentru ID, CT_STRING (alocat dinamic
        int i; // folosit pentru CT_INT, CT_CHAR
        double r; // folosit pentru CT_REAL
    };
    int line; // linia din fisierul de intrare
    struct _Token *next; // inlantuire la urmatorul AL
}Token;

#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");

Token *tokens = NULL;
Token *lastToken = NULL;
int line = 1;
char *pCrtCh;

struct Symbol;typedef struct Symbol Symbol;

typedef enum{
	TB_INT,TB_DOUBLE,TB_CHAR,TB_VOID,TB_STRUCT
	}TypeBase;
typedef struct{
	TypeBase tb;
	Symbol *s;
	int n;
	}Type;

int typeSize(Type *t);

typedef enum{
	SK_VAR,SK_PARAM,SK_FN,SK_STRUCT
	}SymKind;

struct Symbol{
	const char *name;
	SymKind kind;
	Type type;
	Symbol *owner;
	Symbol *next;
	union{
		int varIdx;
		int paramIdx;
		Symbol *structMembers;
		struct{
			Symbol *params;
			Symbol *locals;
			void(*extFnPtr)();
			int instrIdx;
			}fn;
		};
	};

Symbol *newSymbol(const char *name,SymKind kind);
Symbol *dupSymbol(Symbol *symbol);
Symbol *addSymbolToList(Symbol **list,Symbol *s);
int symbolsLen(Symbol *list);
void freeSymbol(Symbol *s);

typedef struct _Domain{
	struct _Domain *parent;
	Symbol *symbols;
	}Domain;

extern Domain *symTable;

Domain *pushDomain();
void dropDomain();
void showDomain(Domain *d,const char *name);
Symbol *findSymbolInDomain(Domain *d,const char *name);
Symbol *findSymbol(const char *name);
Symbol *addSymbolToDomain(Domain *d,Symbol *s);

extern char *globalMemory;
extern int nGlobalMemory;

int allocInGlobalMemory(int nBytes);

typedef struct{
	Type type;
	bool lval;
	bool ct;
	}Ret;
bool canBeScalar(Ret* r);
bool convTo(Type *src,Type *dst);
bool arithTypeTo(Type *t1,Type *t2,Type *dst);
Symbol *findSymbolInList(Symbol *list,const char *name);

enum{ OP_HALT	// incheie executia codului
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
	,OP_DROP	};			// sterge valoarea din varful stivei

typedef union{ // o valoare de pe stiva sau argumentul unei instructiuni
	int i;			// valori intregi si indecsi
	double f;		// valori float
	void *p;		// pointeri
	void(*extFnPtr)();		// pointer la o functie externa
	}Val;
typedef struct{ // o instructiune a masinii virtuale
	int op;		// opcode: OP_*
	Val arg;
	}Instr;

Instr *instructions=NULL;
int nInstructions=0;
Val stack[10000];		// stiva
Val *SP;		// Stack pointer - varful stivei - indica intotdeauna valoarea din varful stivei

Token *addTk(int code){
    Token *tk;
    SAFEALLOC(tk,Token)
    tk->code=code;
    tk->line=line;
    tk->next=NULL;
    if(lastToken){
        lastToken->next=tk;
    }else{
        tokens=tk;
    }
    lastToken=tk;
    return tk;
}

void err(const char *fmt,...){
    va_list va;
    va_start(va,fmt);
    fprintf(stderr,"error: ");
    vfprintf(stderr,fmt,va);
    fputc('\n',stderr);
    va_end(va);
    exit(-1);
}

void tkerr(const Token *tk,const char *fmt,...){
    va_list va;
    va_start(va,fmt);
    fprintf(stderr,"error in line %d: ",tk->line);
    vfprintf(stderr,fmt,va);
    fputc('\n',stderr);
    va_end(va);
    exit(-1);
}

char *createString(char *pStartCh, char *pCrtCh){
    char *mystr = NULL;
    if((mystr=(char*)malloc((pCrtCh - pStartCh)*sizeof(char)))==NULL)err("not enough memory");
    return mystr;
}

int getNextToken(){
    int state=0,nCh, nCh1;
    char ch;
    int reconstituire_int = 0;
    double reconstituire_real = 0;
    const char *pStartCh;
    char *pStartCh1;
    char *startE = NULL;
    Token *tk;
    for(;;){ // bucla infinita
        ch=*pCrtCh;
        switch(state){
        case 0: // testare tranzitii posibile din starea 0
            if (ch == '+'){ pCrtCh++; state = 1;
            }else if (ch == '-'){ pCrtCh++; state = 2;
            }else if (ch == '*'){ pCrtCh++; state = 3;
            }else if (ch == '*'){ pCrtCh++; state = 3;
            }else if (ch == '/'){ pCrtCh++; state = 4;
            }else if (ch == '.'){ pCrtCh++; state = 7;
            }else if (ch == '&'){ pCrtCh++; state = 8;
            }else if (ch == '|'){ pCrtCh++; state = 10;
            }else if (ch == '!'){ pCrtCh++; state = 12;
            }else if (ch == '='){ pCrtCh++; state = 15;
            }else if (ch == '<'){ pCrtCh++; state = 18;
            }else if (ch == '>'){ pCrtCh++; state = 21;
            }else if (ch == ','){ pCrtCh++; state = 24;
            }else if (ch == ';'){ pCrtCh++; state = 25;
            }else if (ch == '('){ pCrtCh++; state = 26;
            }else if (ch == ')'){ pCrtCh++; state = 27;
            }else if (ch == '['){ pCrtCh++; state = 28;
            }else if (ch == ']'){ pCrtCh++; state = 29;
            }else if (ch == '{'){ pCrtCh++; state = 30;
            }else if (ch == '}'){ pCrtCh++; state = 31;
            }else if (ch == '\0'){ pCrtCh++; state = 32;
            }else if(isalpha(ch)||ch=='_'){ pCrtCh++; state=33; // consuma caracterul si trece la noua stare
                pStartCh=pCrtCh - 1; // memoreaza inceputul ID-ului
            }else if(ch==' '||ch=='\r'||ch=='\t'){ pCrtCh++; // consuma caracterul si ramane in starea 0
            }else if(ch=='\n'){ // tratat separat pentru a actualiza linia curenta
                line++; pCrtCh++;
            }else if (isdigit(ch)){ pCrtCh++; reconstituire_int = ch - '0'; startE = pCrtCh - 1 ;state = 35;
            }else if (ch == '\''){ pCrtCh++; state = 43;
            }else if (ch == '"'){ pCrtCh++; state = 46; pStartCh1 = pCrtCh - 1;
            }else if(ch==0){ tk = addTk(END); return END; // sfarsit de sir
            }else tkerr(addTk(END),"caracter invalid");
            break;

        case 1: tk = addTk(ADD); return ADD;
        case 2: tk = addTk(SUB); return SUB;
        case 3: tk = addTk(MUL); return MUL;
        case 4:
            if (ch == '/'){ pCrtCh++; state = 6;
            } else state = 5;
            break;
        case 5: tk = addTk(DIV); return DIV;
        case 6:
            if (ch != '\n' && ch != '\r' && ch != '\t') { pCrtCh++; state = 6; }
            else state = 0;
            break;
        case 7: tk = addTk(DOT); return DOT;
        case 8:
            if (ch == '&'){ pCrtCh++; state = 9;
            } else tkerr(addTk(END),"lipseste inca un &");
            break;
        case 9: tk = addTk(AND); return AND;
        case 10:
            if (ch == '|'){ pCrtCh++; state = 9;
            } else tkerr(addTk(END),"lipseste inca un |");
            break;
        case 11: tk = addTk(OR); return OR;
        case 12:
            if (ch == '='){ pCrtCh++; state = 14;
            } else state = 13;
            break;
        case 13: tk = addTk(NOT); return NOT;
        case 14: tk = addTk(NOTEQ); return NOTEQ;
        case 15:
            if (ch == '='){ pCrtCh++; state = 17;
            } else state = 16;
            break;
        case 16: tk = addTk(ASSIGN); return ASSIGN;
        case 17: tk = addTk(EQUAL); return EQUAL;
        case 18:
            if (ch == '='){ pCrtCh++; state = 20;
            } else state = 19;
            break;
        case 19: tk = addTk(LESS); return LESS;
        case 20: tk = addTk(LESSEQ); return LESSEQ;
        case 21:
            if (ch == '='){ pCrtCh++; state = 23;
            } else state = 22;
            break;
        case 22: tk = addTk(GREATER); return GREATER;
        case 23: tk = addTk(GREATEREQ); return GREATEREQ;
        case 24: tk = addTk(COMMA); return COMMA;
        case 25: tk = addTk(SEMICOLON); return SEMICOLON;
        case 26: tk = addTk(LPAR); return LPAR;
        case 27: tk = addTk(RPAR); return RPAR;
        case 28: tk = addTk(LBRACKET); return LBRACKET;
        case 29: tk = addTk(RBRACKET); return RBRACKET;
        case 30: tk = addTk(LACC); return LACC;
        case 31: tk = addTk(RACC); return RACC;
        case 32: tk = addTk(END); return END;
        case 33:
            if(isalnum(ch)||ch=='_'){ pCrtCh++; state=33; // consuma caracterul si trece la noua stare
            } else state = 34;
            break;
        case 34:
            nCh = pCrtCh-pStartCh; // lungimea cuvantului gasit
            char *cuv;
            if((cuv=(char*)malloc((nCh + 2)*sizeof(char)))==NULL)err("not enough memory");
            int pas = 0;
            for(char* i = pStartCh; i < pCrtCh; i++){ cuv[pas++] = *i; }
            cuv[pas] = '\0';
            // teste cuvinte cheie
            if(nCh == 5&&!memcmp(pStartCh, "break", 5)) {tk = addTk(BREAK); return BREAK; }
            else if(nCh == 4 && !memcmp(pStartCh, "char", 4)){tk = addTk(CHAR); return CHAR; }
            else if(nCh == 4 && !memcmp(pStartCh, "else", 4)){tk = addTk(ELSE); return ELSE; }
            else if(nCh == 4 && !memcmp(pStartCh, "void", 4)){tk = addTk(VOID); return VOID; }
            else if(nCh == 3 && !memcmp(pStartCh, "for", 3)){tk = addTk(FOR); return FOR; }
            else if(nCh == 3 && !memcmp(pStartCh, "int", 3)){tk = addTk(INT); return INT; }
            else if(nCh == 6 && !memcmp(pStartCh, "double", 6)){tk = addTk(DOUBLE); return DOUBLE; }
            else if(nCh == 6 && !memcmp(pStartCh, "return", 6)){tk = addTk(RETURN); return RETURN; }
            else if(nCh == 6 && !memcmp(pStartCh, "struct", 6)){tk = addTk(STRUCT); return STRUCT; }
            else if(nCh == 5 && !memcmp(pStartCh, "while", 5)){tk = addTk(WHILE); return WHILE; }
            else if(nCh == 2 && !memcmp(pStartCh, "if", 2)){tk = addTk(IF); return IF; }
            else{ // daca nu este un cuvant cheie, atunci e un ID
                tk = addTk(ID); tk->text = cuv; return ID;
            }
        case 35:
            if (isdigit(ch)){ pCrtCh++; state = 35;
            reconstituire_int = reconstituire_int * 10 + (ch - '0');
            } else if(ch =='.'){ pCrtCh++; state = 37;
            } else if(ch == 'e' || ch == 'E'){ state = 39; pCrtCh++;
            } else state = 36;
            break;
        case 36: tk = addTk(CT_INT); tk->i = reconstituire_int; reconstituire_int = 0; return CT_INT;
        case 37:
            if (isdigit(ch)){ pCrtCh++; state = 38;
            reconstituire_int = reconstituire_int * 10 + (ch - '0');
            reconstituire_real++;
            } else tkerr(addTk(END),"lipsa parte zecimala");
            break;
        case 38:
            if (isdigit(ch)){ pCrtCh++; state = 38;
            reconstituire_int = reconstituire_int * 10 + (ch - '0');
            reconstituire_real++;
            } else if(ch == 'e' || ch == 'E'){ pCrtCh++; state = 39;
            } else state = 42;
            break;
        case 39:
            if(ch == '+' || ch == '-'){ pCrtCh++; state = 40;
            } else state = 40;
            break;
        case 40:
            if(isdigit(ch)){ pCrtCh++; state = 41;
            } else tkerr(addTk(END),"caracter invalid");
            break;
        case 41:
            if(isdigit(ch)){ pCrtCh++; state = 41;
            } else {
                char *stopstring;
                double x = strtod(startE, &stopstring);
                tk = addTk(CT_REAL); tk->r = x; return CT_REAL;
            }
            break;
        case 42: tk = addTk(CT_REAL); double d = reconstituire_int;
            while(reconstituire_real--){ d /= 10; } tk->r = d;
            reconstituire_int = reconstituire_real = 0;
            return CT_REAL;
        case 43:
            if (ch != '\''){ pCrtCh++; state = 44;
            } else tkerr(addTk(END),"caracter invalid");
            break;
        case 44:
            if (ch == '\''){ pCrtCh++; state = 45;
            } else tkerr(addTk(END),"caracter invalid");
            break;
        case 45: tk = addTk(CT_CHAR); tk->i = *(pCrtCh - 2); return CT_CHAR;
        case 46:
            if (ch != '"'){ pCrtCh++; state = 46;
            } else if(ch == '"'){ pCrtCh++; state = 47;
            } else tkerr(addTk(END),"caracter invalid");
            break;
        case 47:
            nCh1 = pCrtCh-pStartCh1; // lungimea cuvantului gasit
            char *cuvs;
            if((cuvs=(char*)malloc((nCh1 + 2)*sizeof(char)))==NULL)err("not enough memory");
            int pas1 = 0;
            for(char* i = pStartCh1 + 1; i < pCrtCh - 1; i++){ cuvs[pas1++] = *i; }
            cuvs[pas1] = '\0';
            tk = addTk(CT_STRING); tk->text = cuvs; return CT_STRING;
        default: printf("stare necunoscuta"); break;
        }
    }
}

void afisare(Token *tok){
    int code = tok->code;
    switch(code){
        case 0: printf("ID:%s ", tok->text); break;
        case 1: printf("BREAK "); break;
        case 2: printf("CHAR "); break;
        case 3: printf("DOUBLE "); break;
        case 4: printf("ELSE "); break;
        case 5: printf("FOR "); break;
        case 6: printf("IF "); break;
        case 7: printf("INT "); break;
        case 8: printf("RETURN ");break;
        case 9: printf("STRUCT ");break;
        case 10: printf("VOID ");break;
        case 11: printf("WHILE ");break;
        case 12: printf("CT_INT:%d ", tok->i);break;
        case 13: printf("CT_REAL:%lf ", tok->r);break;
        case 14: printf("CT_CHAR:%c ", tok->i);break;
        case 15: printf("CT_STRING:%s ", tok->text);break;
        case 16: printf("COMMA ");break;
        case 17: printf("SEMICOLON ");break;
        case 18: printf("LPAR ");break;
        case 19: printf("RPAR ");break;
        case 20: printf("LBRACKET ");break;
        case 21: printf("RBRACKET ");break;
        case 22: printf("LACC ");break;
        case 23: printf("RACC ");break;
        case 24: printf("END ");break;
        case 25: printf("ADD ");break;
        case 26: printf("SUB ");break;
        case 27: printf("MUL ");break;
        case 28: printf("DIV ");break;
        case 29: printf("DOT ");break;
        case 30: printf("AND ");break;
        case 31: printf("OR ");break;
        case 32: printf("NOT ");break;
        case 33: printf("ASSIGN ");break;
        case 34: printf("EQUAL ");break;
        case 35: printf("NOTEQ ");break;
        case 36: printf("LESS ");break;
        case 37: printf("LESSEQ ");break;
        case 38: printf("GREATER ");break;
        case 39: printf("GREATEREQ ");break;
    }
}

Token *iTk;
Token *consumeTk;
Domain *symTable=NULL;
char *globalMemory=NULL;
int nGlobalMemory=0;
Symbol *owner = NULL;

bool expr();

bool consume(int code){
    if(iTk->code == code){
        consumeTk = iTk;
        iTk = iTk->next;
        return true;
    }
    return false;
}

///exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
///            | CT_INT | CT_REAL | CT_CHAR | CT_STRING | LPAR expr RPAR
bool exprPrimary(Ret *r){
    Token *start = iTk;
    if(consume(ID)){
        Token* tkName = consumeTk;
        Symbol* s = findSymbol(tkName->text);
        if (!s) tkerr(iTk, "undefined id: %s", tkName->text);
        if (consume(LPAR)){
            if (s->kind != SK_FN) tkerr(iTk, "only a function can be called");
            Ret rArg;
            Symbol* param = s->fn.params;
            if(expr(&rArg)){
                if (!param) tkerr(iTk, "too many arguments in function call");
                if (!convTo(&rArg.type, &param->type)) tkerr(iTk, "in call, cannot convert the argument type to the parameter type");
                addRVal(rArg.lval,&rArg.type);
                insertConvIfNeeded(nInstructions,&rArg.type,&param->type);
                while(consume(COMMA)){
                    if(expr(&rArg)){
                        if (!param) tkerr(iTk, "too many arguments in function call");
                        if (!convTo(&rArg.type, &param->type)) tkerr(iTk, "in call, cannot convert the argument type to the parameter type");
                        addRVal(rArg.lval,&rArg.type);
                        insertConvIfNeeded(nInstructions,&rArg.type,&param->type);
                    }
                    else tkerr(iTk,"lipseste o expresie dupa virgula");
                }
            }
            if(consume(RPAR)){
                if (param) tkerr(iTk, "too few arguments in function call");
                *r = (Ret){ s->type,false,true };
                if(s->fn.extFnPtr){
                    int posCallExt=addInstr(OP_CALL_EXT);
                    instructions[posCallExt].arg.extFnPtr=s->fn.extFnPtr;
                }else{ addInstrWithInt(OP_CALL,s->fn.instrIdx); }
                return true;
            } else tkerr(iTk,"lipseste o )");
        }
        if (s->kind == SK_FN) tkerr(iTk, "a function can only be called");
        *r = (Ret){ s->type,true,s->type.n >= 0 };
        if(s->kind==SK_VAR){
            if(s->owner==NULL){ // variabile globale
                addInstrWithInt(OP_ADDR,s->varIdx);
            }else{ // variabile locale
                switch(s->type.tb){
                    case TB_INT:addInstrWithInt(OP_FPADDR_I,s->varIdx+1);break;
                    case TB_DOUBLE:addInstrWithInt(OP_FPADDR_F,s->varIdx+1);break;
                }
            }
        }
        if(s->kind==SK_PARAM){
            switch(s->type.tb){
                case TB_INT:
                addInstrWithInt(OP_FPADDR_I,s->paramIdx-symbolsLen(s->owner->fn.params)-1); break;
                case TB_DOUBLE:
                addInstrWithInt(OP_FPADDR_F,s->paramIdx-symbolsLen(s->owner->fn.params)-1); break;
            }
        }
        return true;
    }
    if(consume(CT_INT)){*r = (Ret){ {TB_INT,NULL,-1},false,true };
        addInstrWithInt(OP_PUSH_I,iTk->i); return true;}
    if(consume(CT_REAL)){ *r = (Ret){ {TB_DOUBLE,NULL,-1},false,true };
        addInstrWithDouble(OP_PUSH_F,iTk->r); return true;}
    if(consume(CT_CHAR)){*r = (Ret){ {TB_CHAR,NULL,-1},false,true }; return true;}
    if(consume(CT_STRING)){*r = (Ret){ {TB_CHAR,NULL,0},false,true }; return true;}
    if(consume(LPAR)){
        if (expr(r)){
            if(consume(RPAR)){
                return true;
            }else tkerr(iTk,"lipseste o ) dupa expresie");
        }else tkerr(iTk,"lipseste o expresie dupa (");
    }
    iTk = start;
    return false;
}

///exprPostfix: exprPostfix LBRACKET expr RBRACKET | exprPostfix DOT ID | exprPrimary
///exprPostfix: exprPrimary exprPostfixPrim
///exprPostfixPrim: LBRACKET expr RBRACKET exprPostfixPrim | DOT ID exprPostfixPrim | epsilon
bool exprPostfixPrim(Ret *r){
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
                } else tkerr(iTk,"lipseste o expresie de postfixare");
            } else tkerr(iTk,"lipseste o ]");
        } else tkerr(iTk,"lipseste o expresie dupa [");
    }
    if(consume(DOT)){
        if(consume(ID)){
            Token* atName = consumeTk;
            if (r->type.tb != TB_STRUCT) tkerr(iTk, "a field can only be selected from a struct");
            Symbol* s = findSymbolInList(r->type.s->structMembers, atName->text);
            if (!s) tkerr(iTk, "the structure %s does not have a field %s",r->type.s->name,atName->text);
            * r = (Ret){ s->type,true,s->type.n >= 0 };
            if(exprPostfixPrim(r)){
                return true;
            } else tkerr(iTk,"lipseste o expresie de postfixare");
        } else tkerr(iTk,"lipseste un nume dupa .");
    }
    return true;
}
bool exprPostfix(Ret *r){
    Token *start = iTk;
    if(exprPrimary(r)){
        if(exprPostfixPrim(r)){
            return true;
        } else tkerr(iTk,"lipseste o expresie de postfixare");
    }
    iTk = start;
    return false;
}

///exprUnary: (SUB | NOT) exprUnary | exprPostfix
bool exprUnary(Ret *r){
    Token *start = iTk;
    if(consume(SUB)){
        if(exprUnary(r)){
            if (!canBeScalar(r)) tkerr(iTk, "unary - must have a scalar operand");
            r->lval = false;
            r->ct = true;
            return true;
        } else tkerr(iTk,"lipseste o expresie unara de dupa semnul -");
    }
    if (consume(NOT)){
        if(exprUnary(r)){
            if (!canBeScalar(r)) tkerr(iTk, "unary - must have a scalar operand");
            r->lval = false;
            r->ct = true;
            return true;
        } else tkerr(iTk,"lipseste o expresie unara de dupa semnul !");
    }
    if(exprPostfix(r)){return true;}
    iTk = start;
    return false;
}

bool typeBase();
bool arrayDecl();

///exprCast: LPAR typeBase arrayDecl? RPAR exprCast | exprUnary
bool exprCast(Ret *r){
    Token *start = iTk;
    if(consume(LPAR)){
        Type t;
        Ret op;
        if(typeBase(&t)){
            if(arrayDecl(&t)){}
            if (consume(RPAR)){
                if(exprCast(&op)){
                    if (t.tb == TB_STRUCT) tkerr(iTk, "cannot convert to a struct type");
                    if (op.type.tb == TB_STRUCT) tkerr(iTk, "cannot convert a struct");
                    if (op.type.n >= 0 && t.n < 0) tkerr(iTk, "an array can be converted only to another array");
                    if (op.type.n < 0 && t.n >= 0) tkerr(iTk, "a scalar can be converted only to another scalar");
                    *r = (Ret){ t,false,true };
                    return true;
                } else tkerr(iTk,"lipseste o expresie dupa )");
            }else tkerr(iTk,"lipseste ) dupa specificarea tipului");
        } else tkerr(iTk,"lipseste specificarea tipului dupa ( ");
    }
    if(exprUnary(r)){return true;}
    iTk = start;
    return false;
}

///exprMul: exprMul (MUL | DIV) exprCast | exprCast
///exprMul: exprCast exprMulPrim
///exprMulPrim: (MUL | DIV) exprCast exprMulPrim | epsilon
bool exprMulPrim(Ret *r){
    if(consume(MUL) ){
        Ret right;
        Token *op = consumeTk;
        int posLeft=nInstructions;
        addRVal(r->lval,&r->type);
        if(exprCast(&right)){
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(iTk, "inavlid operand type for *\n");
            insertConvIfNeeded(posLeft,&r->type,&tDst);
            insertConvIfNeeded(nInstructions,&right.type,&tDst);
            switch(op->code){
                case MUL:
                    switch(tDst.tb){
                        case TB_INT:addInstr(OP_MUL_I);break;
                        case TB_DOUBLE:addInstr(OP_MUL_F);break;
                    }
                    break;
                case DIV:
                    switch(tDst.tb){
                        case TB_INT:addInstr(OP_DIV_I);break;
                        case TB_DOUBLE:addInstr(OP_DIV_F);break;
                    }
                    break;
            }
            if (exprMulPrim(r)){
                return true;
            } else tkerr(iTk,"lipseste o expresie pentru multiplicare");
        } else tkerr(iTk,"lipseste expresia de dupa *");
    }
    if (consume(DIV)){
        Ret right;
        Token *op = consumeTk;
        int posLeft=nInstructions;
        addRVal(r->lval,&r->type);
        if(exprCast(&right)){
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(iTk, "inavlid operand type for /\n");
            insertConvIfNeeded(posLeft,&r->type,&tDst);
            insertConvIfNeeded(nInstructions,&right.type,&tDst);
            switch(op->code){
                case MUL:
                    switch(tDst.tb){
                        case TB_INT:addInstr(OP_MUL_I);break;
                        case TB_DOUBLE:addInstr(OP_MUL_F);break;
                    }
                    break;
                case DIV:
                    switch(tDst.tb){
                        case TB_INT:addInstr(OP_DIV_I);break;
                        case TB_DOUBLE:addInstr(OP_DIV_F);break;
                    }
                    break;
            }
            if (exprMulPrim(r)){
                return true;
            } else tkerr(iTk,"lipseste o expresie pentru multiplicare");
        } else tkerr(iTk,"lipseste expresia de dupa /");
    }
    return true;
}
bool exprMul(Ret *r){
    Token *start = iTk;
    if(exprCast(r)){
        if(exprMulPrim(r)){
            return true;
        } else tkerr(iTk,"lipseste o expresie pentru multiplicare");
    }
    iTk = start;
    return false;
}

///exprAdd: exprAdd (ADD | SUB) exprMul | exprMul
///exprAdd: exprMul exprAddPrim
///exprAddPrim: (ADD | SUB) exprMul exprAddPrim | epsilon
bool exprAddPrim(Ret *r){
    Token *op;
    if(consume(ADD)){
        op = consumeTk;
        Ret right;
        int posLeft=nInstructions;
        addRVal(r->lval,&r->type);
        if(exprMul(&right)){
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(iTk, "inavlid operand type for +\n");
            addRVal(right.lval,&right.type);
            insertConvIfNeeded(posLeft,&r->type,&tDst);
            insertConvIfNeeded(nInstructions,&right.type,&tDst);
            switch(op->code){
                case ADD:
                    switch(tDst.tb){
                        case TB_INT:addInstr(OP_ADD_I);break;
                        case TB_DOUBLE:addInstr(OP_ADD_F);break;
                    }
                break;
                case SUB:
                    switch(tDst.tb){
                        case TB_INT:addInstr(OP_SUB_I);break;
                        case TB_DOUBLE:addInstr(OP_SUB_F);break;
                    }
                break;
            }
            if (exprAddPrim(r)){
                return true;
            }
        } else tkerr(iTk,"lipseste o expresie pentru adunare");
    }
    if(consume(SUB)){
        op = consumeTk;
        Ret right;
        int posLeft=nInstructions;
        addRVal(r->lval,&r->type);
        if(exprMul(&right)){
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(iTk, "inavlid operand type for -\n");
            addRVal(right.lval,&right.type);
            insertConvIfNeeded(posLeft,&r->type,&tDst);
            insertConvIfNeeded(nInstructions,&right.type,&tDst);
            switch(op->code){
                case ADD:
                    switch(tDst.tb){
                        case TB_INT:addInstr(OP_ADD_I);break;
                        case TB_DOUBLE:addInstr(OP_ADD_F);break;
                    }
                break;
                case SUB:
                    switch(tDst.tb){
                        case TB_INT:addInstr(OP_SUB_I);break;
                        case TB_DOUBLE:addInstr(OP_SUB_F);break;
                    }
                break;
            }
            if (exprAddPrim(r)){
                return true;
            }
        } else tkerr(iTk,"lipseste o expresie pentru scadere");
    }
    return true;
}
bool exprAdd(Ret *r){
    Token *start = iTk;
    if(exprMul(r)){
        if(exprAddPrim(r)){
            return true;
        } else tkerr(iTk,"lipseste o expresie pentru adunare");
    }
    iTk = start;
    return false;
}

///exprRel: exprRel (LESS | LESSEQ | GREATER | GREATEREQ) exprAdd | exprAdd
///exprRel: exprAdd exprRelPrim
///exprRelPrim: (LESS | LESSEQ | GREATER | GREATEREQ) exprAdd exprRelPrim | epsilon
bool exprRelPrim(Ret *r){
    Token *op;
    if(consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)){
        op = consumeTk;
        Ret right;
        int posLeft=nInstructions;
        addRVal(r->lval,&r->type);
        if(exprAdd(&right)){
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(iTk, "inavlid operand type for <, >=, <, <=\n");
            addRVal(right.lval,&right.type);
            insertConvIfNeeded(posLeft,&r->type,&tDst);
            insertConvIfNeeded(nInstructions,&right.type,&tDst);
            switch(op->code){
                case LESS:
                    switch(tDst.tb){
                        case TB_INT:addInstr(OP_LESS_I);break;
                        case TB_DOUBLE:addInstr(OP_LESS_F);break;
                    }
                break;
            }

            if (exprRelPrim(r)){
                return true;
            } else tkerr(iTk,"lipseste o expresie pentru relatie");
        } else tkerr(iTk,"lipseste o expresie pentru >, >=, <, <=");
    }
    return true;
}
bool exprRel(Ret *r){
    Token *start = iTk;
    if(exprAdd(r)){
        if(exprRelPrim(r)){
            return true;
        } else tkerr(iTk,"lipseste o expresie pentru relatie");
    }
    iTk = start;
    return false;
}

///exprEq: exprEq (EQUAL | NOTEQ) exprRel | exprRel
///exprEq: exprRel exprEqPrim
///exprEqPrim: (EQUAL | NOTEQ) exprRel exprEqPrim | epsilon
bool exprEqPrim(Ret *r){
    if(consume(EQUAL) || consume(NOTEQ)){
        Ret right;
        if(exprRel(&right)){
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(iTk, "inavlid operand type for ==, !=\n");
            *r = (Ret){ {TB_INT, NULL, -1}, false, true };
            if (exprEqPrim(r)){
                return true;
            } else tkerr(iTk,"lipseste o expresie pentru egalitate");
        } else tkerr(iTk,"lipseste o expresie pentru relatie");
    }
    return true;
}
bool exprEq(Ret *r){
    Token *start = iTk;
    if(exprRel(r)){
        if(exprEqPrim(r)){
            return true;
        } else tkerr(iTk,"lipseste o expresie pentru egalitate/inegalitate");
    }
    iTk = start;
    return false;
}

///exprAnd: exprAnd AND exprEq | exprEq
///exprAnd: exprEq exprAndPrim
///exprAndPrim: AND exprEq exprAndPrim | epsilon
bool exprAndPrim(Ret *r){
    if(consume(AND)){
        Ret right;
        if(exprEq(&right)){
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(iTk, "inavlid operand type for &&\n");
            *r = (Ret){ {TB_INT, NULL, -1}, false, true };
            if (exprAndPrim(r)){
                return true;
            } else tkerr(iTk,"lipseste o expresie pentru \"and\"");
        } else tkerr(iTk,"lipseste o expresie dupa &&");
    }
    return true;
}
bool exprAnd(Ret *r){
    Token *start = iTk;
    if(exprEq(r)){
        if(exprAndPrim(r)){
            return true;
        } else tkerr(iTk,"lipseste o expresie pentru \"and\"");
    }
    iTk = start;
    return false;
}

///exprOr: exprOr OR exprAnd | exprAnd
///exprOr: exprAnd exprOrPrim
///exprOrPrim: OR exprAnd exprOrPrim | epsilon
bool exprOrPrim(Ret *r){
    if(consume(OR)){
        Ret right;
        if(exprAnd(&right)){
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(iTk, "inavlid operand type for ||\n");
            *r = (Ret){ {TB_INT, NULL, -1}, false, true };
            if (exprOrPrim(r)){
                return true;
            } else tkerr(iTk,"lipseste o expresie pentru \"or\"");
        } else tkerr(iTk,"lipseste o expresie pentru \"or\"");
    }
    return true;
}
bool exprOr(Ret *r){
    Token *start = iTk;
    if(exprAnd(r)){
        if(exprOrPrim(r)){
            return true;
        } else tkerr(iTk,"lipseste o expresie pentru \"or\"");
    }
    iTk = start;
    return false;
}

///exprAssign: exprUnary ASSIGN exprAssign | exprOr
bool exprAssign(Ret *r){
    Token *start = iTk;
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
                addRVal(r->lval,&r->type);
                insertConvIfNeeded(nInstructions,&r->type,&rDst.type);
                switch(rDst.type.tb){
                    case TB_INT:addInstr(OP_STORE_I);break;
                    case TB_DOUBLE:addInstr(OP_STORE_F);break;
                }
                return true;
            } else {tkerr(iTk,"lipseste dupa semnul = o expresie de atribuire");}
        }
        iTk = start;
    }
    if(exprOr(r)){return true;}
    iTk = start;
    return false;
}

///expr: exprAssign
bool expr(Ret *r){
    Token *start = iTk;
    if(exprAssign(r)){return true;}
    iTk = start;
    return false;
}

bool varDef();
bool stm();

///stmCompound: LACC ( varDef | stm )* RACC
bool stmCompound(bool newDomain){
    Token *start = iTk;
    if(consume(LACC)){
        if(newDomain) pushDomain();
        while(varDef() || stm()){ }
        if(consume(RACC)){
            if(newDomain) dropDomain();
            return true;
        } else tkerr(iTk,"lipseste }");
    }
    iTk = start;
    return false;
}

///   stm: stmCompound | IF LPAR expr RPAR stm ( ELSE stm )?
///                    | WHILE LPAR expr RPAR stm
///                    | FOR LPAR expr? SEMICOLON expr? SEMICOLON expr? RPAR stm
///                    | BREAK SEMICOLON
///                    | RETURN expr? SEMICOLON
///                    | expr? SEMICOLON
bool stm(){
    Token *start = iTk;
    Ret rInit, rCond, rStep, rExpr;
    if(stmCompound(true)){
        return true;
    }
    if(consume(IF)){
        if(consume(LPAR)){
            if(expr(&rCond)){
                if (!canBeScalar(&rCond)) tkerr(iTk, "the if condition must be a scalar value\n");
                if(consume(RPAR)){
                    addRVal(rCond.lval,&rCond.type);
                    Type intType={TB_INT,NULL,-1};
                    insertConvIfNeeded(nInstructions,&rCond.type,&intType);
                    int posJF=addInstr(OP_JF);
                    if(stm()){
                        if(consume(ELSE)){
                            int posJMP=addInstr(OP_JMP);
                            instructions[posJF].arg.i=nInstructions;
                            if(stm()){
                                instructions[posJMP].arg.i=nInstructions;
                                return true;
                            }else tkerr(iTk,"lipseste corpul de dupa else");
                        }
                        instructions[posJF].arg.i = nInstructions;
                        return true;
                    } else tkerr(iTk,"lipseste corpul de dupa )");
                } else tkerr(iTk,"lipseste )");
            } else tkerr(iTk,"lipseste o expresie dupa (");
        } else tkerr(iTk,"lipseste (");
    }
    if(consume(WHILE)){
        int posCond=nInstructions;
        if(consume(LPAR)){
            if(expr(&rCond)){
                if (!canBeScalar(&rCond)) tkerr(iTk, "the while condition must be a scalar value\n");
                if(consume(RPAR)){
                    addRVal(rCond.lval,&rCond.type);
                    Type intType={TB_INT,NULL,-1};
                    insertConvIfNeeded(nInstructions,&rCond.type,&intType);
                    int posJF=addInstr(OP_JF);
                    if(stm()){
                        addInstrWithInt(OP_JMP,posCond);
                        instructions[posJF].arg.i=nInstructions;
                        return true;
                    } else tkerr(iTk,"lipseste corpul din interiorul lui while");
                } else tkerr(iTk,"lipseste )");
            } else tkerr(iTk,"lipseste expresia de dupa (");
        } else tkerr(iTk,"lipseste (");
    }
    if(consume(FOR)){
        if(consume(LPAR)){
            if(expr(&rInit)){}
            if(consume(SEMICOLON)){
                if(expr(&rCond)){
                    if (!canBeScalar(&rCond))
                        tkerr(iTk, "the for condition must be a scalar value\n");
                }
                if (consume(SEMICOLON)){
                    if(expr(&rStep)){}
                    if (consume(RPAR)){
                        if(stm()){
                            return true;
                        } else tkerr(iTk,"lipseste corpul lui for");
                    } else tkerr(iTk,"lipseste )");
                } else tkerr(iTk,"lipseste ; in for");
            } else tkerr(iTk,"lipseste ; in for");
        } else tkerr(iTk,"lipseste (");
    }
    if(consume(BREAK)){
        if(consume(SEMICOLON)){
            return true;
        } else tkerr(iTk,"lipseste ; dupa break");
    }
    if(consume(RETURN)){ //RETURN expr? SEMICOLON
        if(expr(&rExpr)){
            addRVal(rExpr.lval,&rExpr.type);
            insertConvIfNeeded(nInstructions,&rExpr.type,&owner->type);
            addInstrWithInt(OP_RET,symbolsLen(owner->fn.params));
            if (owner->type.tb == TB_VOID) tkerr(iTk, "a void function cannot return value\n");
            if (!canBeScalar(&rExpr)) tkerr(iTk, "the return value must be a scalar value\n");
            if (!convTo(&rExpr.type, &owner->type)) tkerr(iTk, "cannot convert the return expression type to the function return type\n");
        }
        else { addInstr(OP_RET_VOID);
            if (owner->type.tb != TB_VOID) tkerr(iTk, "a non-void function must return a value\n");}
        if(consume(SEMICOLON)){
            return true;
        } else tkerr(iTk,"lipseste ; dupa return");
    }
    if(expr(&rExpr)){
        if(rExpr.type.tb!=TB_VOID)addInstr(OP_DROP);
        if(consume(SEMICOLON)){
            return true;
        } else {tkerr(iTk,"lipseste ;");}
    }
    if(consume(SEMICOLON)){
        return true;
    }
    iTk = start;
    return false;
}

///fnParam: typeBase ID arrayDecl?
bool fnParam(){
    Token *start = iTk;
    Type t;
    if(typeBase(&t)){
        if(consume(ID)){
            Token *tkName = consumeTk;
            if(arrayDecl(&t)){}
            Symbol *param=findSymbolInDomain(symTable,tkName->text);
            if(param)tkerr(iTk,"symbol redefinition: %s",tkName->text);
            param=newSymbol(tkName->text,SK_PARAM);
            param->type=t;
            param->paramIdx=symbolsLen(owner->fn.params);
            addSymbolToDomain(symTable,param);
            addSymbolToList(&owner->fn.params,dupSymbol(param));
            return true;
        } else tkerr(iTk,"lipseste numele de dupa specificarea tipului");
    }
    iTk = start;
    return false;
}

///fnDef: ( typeBase | VOID ) ID LPAR ( fnParam ( COMMA fnParam )* )? RPAR stmCompound
bool fnDef(){
    Token *start = iTk;
    Type t;
    if(consume(VOID)){
        t.tb = TB_VOID;
        if(consume(ID)){
            Token *tkName = consumeTk;
            if(consume(LPAR)){
                Symbol *fn=findSymbolInDomain(symTable,tkName->text);
                if(fn)tkerr(iTk,"symbol redefinition: %s",tkName->text);
                fn=newSymbol(tkName->text,SK_FN);
                fn->type=t;
                addSymbolToDomain(symTable,fn);
                owner=fn;
                pushDomain();
                if(fnParam()){
                    while(consume(COMMA)){
                        if(fnParam()){}
                        else tkerr(iTk,"lipseste parametrul de dupa virgula");
                    }
                }
                if(consume(RPAR)){
                    owner->fn.instrIdx=nInstructions;
                    addInstr(OP_ENTER);
                    if(stmCompound(false)){
                        dropDomain();
                        instructions[owner->fn.instrIdx].arg.i=symbolsLen(owner->fn.locals);
                        if(owner->type.tb==TB_VOID)addInstrWithInt(OP_RET_VOID,symbolsLen(owner->fn.params));
                        return true;
                    } else tkerr(iTk,"lipseste corpul");
                } else tkerr(iTk,"lipseste )");
            } else tkerr(iTk,"lipseste (");
        } else tkerr(iTk,"lipseste numele de dupa specificarea tipului");
    }
    if(typeBase(&t)){
        if(consume(ID)){
            Token *tkName = consumeTk;
            if(consume(LPAR)){
                Symbol *fn=findSymbolInDomain(symTable,tkName->text);
                if(fn)tkerr(iTk,"symbol redefinition: %s",tkName->text);
                fn=newSymbol(tkName->text,SK_FN);
                fn->type=t;
                addSymbolToDomain(symTable,fn);
                owner=fn;
                pushDomain();
                if(fnParam()){
                    while(consume(COMMA)){
                        if(fnParam()){}
                        else tkerr(iTk,"lipseste parametrul de dupa virgula");
                    }
                }
                if(consume(RPAR)){
                    owner->fn.instrIdx=nInstructions;
                    addInstr(OP_ENTER);
                    if(stmCompound(false)){
                        dropDomain();
                        instructions[owner->fn.instrIdx].arg.i=symbolsLen(owner->fn.locals);
                        if(owner->type.tb==TB_VOID)addInstrWithInt(OP_RET_VOID,symbolsLen(owner->fn.params));
                        return true;
                    } else tkerr(iTk,"lipseste corpul");
                } else tkerr(iTk,"lipseste )");
            }
        } else tkerr(iTk,"lipseste numele de dupa specificarea tipului");
    }
    iTk = start;
    return false;
}

///arrayDecl: LBRACKET CT_INT? RBRACKET
bool arrayDecl(Type *t){
    Token *start = iTk;
    if(consume(LBRACKET)){
        if(consume(CT_INT)){
            Token *tkName = consumeTk;
            t->n = tkName->i;
        } else {t->n = 0;}
        if(consume(RBRACKET)){
            return true;
        } else tkerr(iTk,"lipseste ]");
    }
    iTk = start;
    return false;
}

///typeBase: INT | DOUBLE | CHAR | STRUCT ID
bool typeBase(Type *t){
    Token *start = iTk;
    t->n = -1;
    if(consume(INT)){t->tb = TB_INT;return true;}
    if(consume(DOUBLE)){t->tb = TB_DOUBLE;return true;}
    if(consume(CHAR)){t->tb = TB_CHAR;return true;}
    if(consume(STRUCT)){
        if(consume(ID)){
            Token *tkName = consumeTk;
            t->tb = TB_STRUCT;
            t->s = findSymbol(tkName->text);
            if(!t->s) tkerr(iTk,"structura nedefinita: %s", tkName->text);
            return true;
        } else tkerr(iTk,"lipseste un nume pentru declararea structurii");
    }
    iTk = start;
    return false;
}

///varDef: typeBase ID arrayDecl? SEMICOLON
bool varDef(){
    Token *start = iTk;
    Type t;

    if(typeBase(&t)){
        if(consume(ID)){
            Token *tkName = consumeTk;
            if(arrayDecl(&t)){
                if(t.n == 0) tkerr(iTk, "vectorul nu are specificata dimensiunea");
            }
            if (consume(SEMICOLON)){
                Symbol *var=findSymbolInDomain(symTable,tkName->text);
                if(var)tkerr(iTk,"symbol redefinition: %s", tkName->text);
                var=newSymbol(tkName->text,SK_VAR);
                var->type=t;
                var->owner=owner;
                addSymbolToDomain(symTable,var);
                if(owner){
                    switch(owner->kind){
                        case SK_FN:
                            var->varIdx=symbolsLen(owner->fn.locals);
                            addSymbolToList(&owner->fn.locals,dupSymbol(var));
                            break;
                        case SK_STRUCT:
                            var->varIdx=typeSize(&owner->type);
                            addSymbolToList(&owner->structMembers,dupSymbol(var));
                            break;
                    }
                } else{var->varIdx=allocInGlobalMemory(typeSize(&t));}
                return true;
            } else tkerr(iTk,"lipseste ; dupa declaratie");
        } else tkerr(iTk,"lipseste numele de dupa tipul declarat");
    }
    iTk = start;
    return false;
}

///structDef: STRUCT ID LACC varDef* RACC SEMICOLON
bool structDef(){
    Token *start = iTk;
    if(consume(STRUCT)){
        if(consume(ID)){
            Token *tkName = consumeTk;
            if (consume(LACC)){

                Symbol *s = findSymbolInDomain(symTable, tkName->text);
                if(s)tkerr(iTk,"symbol redefinition: %s",tkName->text);
                s = addSymbolToDomain(symTable,newSymbol(tkName->text,SK_STRUCT));
                s->type.tb=TB_STRUCT;
                s->type.s=s;
                s->type.n=-1;
                pushDomain();
                owner=s;

                while(varDef()){}
                if(consume(RACC)){
                    if(consume(SEMICOLON)){
                        owner = NULL;
                        dropDomain();
                        return true;
                    } else tkerr(iTk,"lipseste ;");
                }else tkerr(iTk,"lipseste }");
            }
        } else tkerr(iTk,"lipseste numele tipului struct");
    }
    iTk = start;
    return false;
}

///unit: ( structDef | fnDef | varDef )* END
bool unit(){
    Token *start = iTk;
    addInstr(OP_CALL);
    addInstr(OP_HALT);
    while(structDef() || fnDef() || varDef() ){}
    if(consume(END)){
        Symbol *sm=findSymbol("main");
        if(!sm)tkerr(iTk,"undefined: main");
        instructions[0].arg.i=sm->fn.instrIdx;
        return true;
    }
    iTk = start;
    return false;
}

//-------------------------------------------------------
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
void freeSymbols(Symbol *list){
	for(Symbol *next;list;list=next){
		next=list->next;
		freeSymbol(list);
    }
}
Symbol *newSymbol(const char *name,SymKind kind){
	Symbol *s;
	SAFEALLOC(s,Symbol)
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
Symbol *addSymbolToList(Symbol **list,Symbol *s){
	Symbol *iter=*list;
	if(iter){
		while(iter->next)iter=iter->next;
		iter->next=s;
    }else{ *list=s; }
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

//-----------------------------------------------------------------------------
bool canBeScalar(Ret* r){
	Type* t=&r->type;
	if(t->n>=0)return false;
	if(t->tb==TB_VOID)return false;
	return true;
}
bool convTo(Type *src,Type *dst){
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
		case TB_STRUCT:
			if(dst->tb==TB_STRUCT&&src->s==dst->s)return true;
			return false;
		default:return false;
    }
}
bool arithTypeTo(Type *t1,Type *t2,Type *dst){
	if(t1->n>=0||t2->n>=0)return false;
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

//-------------------------------------------------------
// masina virtuala stiva - instructiunile masinii virtuale
// FORMAT: OP_<name>.<data_type>    // [argument] effect
//		OP_ - prefix comun (operation code)
//		<name> - numele instructiunii
//		<data_type> - daca este prezent, tipul de date asupra caruia actioneaza instructiunea
//			.i - int, .f - double, .c - char, .p - pointer
//		[argument] - daca este prezent, argumentul instructiunii
//		effect - efectul instructiunii

extern Instr *instructions; // vector de instructiuni
extern int nInstructions; // nr de instructiuni
int addInstr(int op);
int insertInstr(int pos,int op);
int addInstrWithInt(int op,int i);
int addInstrWithDouble(int op,double d);
void mvInit();
void run();
void genTestProgram();

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
int popi(){
	if(SP==stack-1)err("trying to pop from empty stack");
	return SP--->i;
}
double popf(){
	if(SP==stack-1)err("trying to pop from empty stack");
	return SP--->f;
}
void pushf(double f){
	if(SP+1==stack+10000)err("trying to push into a full stack");
	(++SP)->f=f;
}
void pushp(void *p){
	if(SP+1==stack+10000)err("trying to push into a full stack");
	(++SP)->p=p;
}
void *popp(){
	if(SP==stack-1)err("trying to pop from empty stack");
	return SP--->p;
}
void put_i(){ printf("=> %d",popi()); }
void put_d(){ printf("=> %f",popf()); }
void mvInit(){
	Symbol *fn=addExtFn("put_i",put_i,(Type){TB_VOID,NULL,-1});
	addFnParam(fn,"i",(Type){TB_INT,NULL,-1});
	Symbol *fn2=addExtFn("put_d",put_d,(Type){TB_VOID,NULL,-1});
	addFnParam(fn2,"i",(Type){TB_INT,NULL,-1});
}
void run(){
	SP=stack-1;
	Val *FP=NULL; Val v;
	Instr *IP=instructions;
	double iTop, iBefore, fTop;
    int iArg;
	void *pTop; void(*extFnPtr)();
	for(;;){
		printf("%03d/%d\t",(int)(IP-instructions),(int)(SP-stack+1));
		switch(IP->op){
			case OP_HALT: printf("HALT"); return;
			case OP_PUSH_I: printf("PUSH.i\t%d",IP->arg.i);
				pushi(IP->arg.i); IP++; break;
			case OP_CALL: pushp(IP+1); printf("CALL\t%d",IP->arg.i);
				IP=instructions+IP->arg.i; break;
			case OP_CALL_EXT: extFnPtr=IP->arg.extFnPtr;
				printf("CALL_EXT\t%p\n",extFnPtr); extFnPtr();
				IP++; break;
			case OP_ENTER: pushp(FP); FP=SP; SP+=IP->arg.i;
				printf("ENTER\t%d",IP->arg.i); IP++; break;
			case OP_RET_VOID: iArg=IP->arg.i;
				printf("RET_VOID\t%d",iArg);
				IP=FP[-1].p; SP=FP-iArg-2; FP=FP[0].p; break;
			case OP_JMP: printf("JMP\t%d",IP->arg.i);
				IP=instructions+IP->arg.i; break;
			case OP_JF: iTop=popf();
				printf("JF\t%d\t// %d",IP->arg.i,iTop);
				IP=iTop ? IP+1 : instructions+IP->arg.i;
				break;
			case OP_FPLOAD: v=FP[IP->arg.i]; pushv(v);
				printf("FPLOAD\t%d\t// i:%d, f:%g",IP->arg.i,v.i,v.f);
				IP++; break;
			case OP_FPSTORE: v=popv(); FP[IP->arg.i]=v;
				printf("FPSTORE\t%d\t// i:%d, f:%g",IP->arg.i,v.i,v.f);
				IP++; break;
			case OP_ADD_I: iTop=popi(); iBefore=popi(); pushi(iBefore+iTop);
				printf("ADD.i\t// %d+%d -> %d",iBefore,iTop,iBefore+iTop);
				IP++; break;
            case OP_ADD_F: iTop=popf(); iBefore=popf(); pushf(iBefore+iTop);
				printf("ADD.f\t// %f+%f -> %f",iBefore,iTop,iBefore+iTop);
				IP++; break;
			case OP_LESS_I: iTop=popi(); iBefore=popi(); pushi(iBefore<iTop);
				printf("LESS.i\t// %d<%d -> %d",iBefore,iTop,iBefore<iTop);
				IP++; break;
            case OP_LESS_F: iTop=popf(); iBefore=popf(); pushf(iBefore<iTop);
				printf("LESS.f\t// %f<%f -> %d",iBefore,iTop,iBefore<iTop);
				IP++; break;
			case OP_RET: v=popv(); iArg=IP->arg.i;
				printf("RET\t%d\t// i:%d, f:%g",iArg,v.i,v.f);
				IP=FP[-1].p; SP=FP-iArg-2; FP=FP[0].p; pushv(v); break;
			case OP_PUSH_F: printf("PUSH.f\t%g",IP->arg.f);
				pushf(IP->arg.f); IP++; break;
			case OP_CONV_F_I: fTop=popf(); pushi((int)fTop);
				printf("CONV.f.i\t// %g -> %d",fTop,(int)fTop);
				IP++; break;
			case OP_LOAD_I: pTop=popp(); pushi(*(int*)pTop);
				printf("LOAD.i\t// *(int*)%p -> %d",pTop,*(int*)pTop);
				IP++; break;
			case OP_STORE_I: iTop=popi(); v=popv(); *(int*)v.p=iTop;
				pushi(iTop); printf("STORE.i\t//*(int*)%p=%d",v.p,iTop);
				IP++; break;
			case OP_FPADDR_I: pTop=&FP[IP->arg.i].i; pushp(pTop);
				printf("FPADDR\t%d\t// %p",IP->arg.i,pTop); IP++; break;
			case OP_SUB_I: iTop=popi(); iBefore=popi(); pushi(iBefore-iTop);
				printf("SUB.i\t// %d-%d -> %d",iBefore,iTop,iBefore-iTop);
				IP++; break;
			case OP_MUL_I: iTop=popi(); iBefore=popi(); pushi(iBefore*iTop);
				printf("MUL.i\t// %d*%d -> %d",iBefore,iTop,iBefore*iTop);
				IP++; break;
			case OP_DROP: popv(); printf("DROP"); IP++; break;
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
	addInstrWithInt(OP_PUSH_I,2);
	int callPos=addInstr(OP_CALL);
	addInstr(OP_HALT);
	instructions[callPos].arg.i=nInstructions;
	addInstrWithInt(OP_ENTER,1);
	// i=0;  (componentele lui for sunt implementate sub forma unui while)
	addInstrWithInt(OP_PUSH_I,0);
	addInstrWithInt(OP_FPSTORE,1);
	// while(i<n){
	int whilePos=addInstrWithInt(OP_FPLOAD,1);
	addInstrWithInt(OP_FPLOAD,-2);
	addInstr(OP_LESS_I);
	int jfAfterFor=addInstr(OP_JF);
	// put_i(i);
	addInstrWithInt(OP_FPLOAD,1);
	Symbol *s=findSymbol("put_i");
	if(!s)err("undefined: put_i");
	int putiCall=addInstr(OP_CALL_EXT);
	instructions[putiCall].arg.extFnPtr=s->fn.extFnPtr;
	// i=i+1
	addInstrWithInt(OP_FPLOAD,1);
	addInstrWithInt(OP_PUSH_I,1);
	addInstr(OP_ADD_I);
	addInstrWithInt(OP_FPSTORE,1);
	// } ( urmatoarea iteratie )
	addInstrWithInt(OP_JMP,whilePos);
	// revenire din functie
	instructions[jfAfterFor].arg.i=nInstructions;
	addInstrWithInt(OP_RET_VOID,1);
	/**addInstrWithDouble(OP_PUSH_F,2.0);
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
	addInstrWithInt(OP_RET_VOID,1);*/
}

//-------------------------------------------------------
// insereaza la pozitia specificata o instructiune de conversie, doar daca este necesar
void insertConvIfNeeded(int pos,Type *srcType,Type *dstType);
// daca lval este true, se genereaza rval din valoarea curenta de pe stiva
void addRVal(bool lval,Type *type);
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

void ASIN(){
    iTk = tokens;
	Token *tok;
	tok = tokens;
	pushDomain();
	if (unit()){}
}

int main(){
    FILE *fp;
    fp = fopen("fis1.txt", "r");
    int count;
    char ch;
	while((ch = fgetc(fp)) != EOF) {count++;} //printf("%c",ch);
	fclose(fp);
	char *strng;
	if((strng=(char*)malloc((count + 1)*sizeof(char)))==NULL)err("not enough memory");
	fp = fopen("fis1.txt", "r");
	int i = 0;
	while((ch = fgetc(fp)) != EOF) {strng[i++] = ch;}
	strng[i] = '\0';
    pCrtCh = strng;
    printf("\n\n");
    while(getNextToken() != END){ }

    iTk = tokens;
	Token *tok;
	tok = tokens;
	while(tok != lastToken){
        printf("linie %d: ", tok->line);
        afisare(tok);
        printf("\n");
        tok = tok->next;
	}
	printf("linie %d: ", tok->line);
	afisare(tok);

	printf("\n\n-------------------------------------------------------------------ASIN-\n\n");

	ASIN(); printf("Syntax ok");

	printf("\n\n----------------------------------------------------------DOMAIN & TYPE-\n\n");

	showDomain(symTable, "global");

	printf("---------------------------------------------------------MASINA VIRTUALA-\n\n");

	/*mvInit();
	genTestProgram();
	run();*/

	dropDomain();

	printf("\n\n------------------------------------------------------------------------\n\n");



	fclose(fp);
	return 0;
}

/*
struct S{
	int n;
	char text[16];
	};

struct S a;
struct S v[10];

void f(char text[],int i,char ch){
	text[i]=ch;
	}

int h(int x,int y){
	if(x>0&&x<y){
		f(v[x].text,y,'#');
		return 1;
		}
	return 0;
	}
*/
