#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include<stdbool.h>

#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL)err("notenough memory");

enum {
    ID, CT_INT, CT_REAL, CT_CHAR, CT_STRING,
    BREAK, CHAR, DOUBLE, ELSE, FOR, IF, INT, RETURN, STRUCT, VOID, WHILE,
    COMMA, SEMICOLON, LPAR, RPAR, LBRACKET, RBRACKET, LACC, RACC, END,
    ADD, SUB, MUL, DIV, DOT, AND, OR, NOT, ASSIGN, EQUAL, NOTEQ, LESS, LESSEQ, GREATER, GREATEREQ
};

typedef struct Atom {
    int code;
    int line;
    union {
        int i;
        double r;
        char* text;
    };
    struct Atom* next;
}Atom;

void err(const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}

void aterr(const Atom* at, const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error in line %d: ", at->line);
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}

Atom* atoms;
Atom* lastAtom;
int line = 1;
char* pch;
char* pStartCh;

Atom* addAtom(int code)
{
    Atom* at;
    SAFEALLOC(at, Atom);
    at->code = code;
    at->line = line;
    at->next = NULL;
    if (lastAtom) {
        lastAtom->next = at;
    }
    else {
        atoms = at;
    }
    lastAtom = at;
    return at;
}

char* createString(const char* start, const char* stop) {
    int n = stop - start;
    char* buffer = (char*)malloc(sizeof(char) * n + 1);
    if (buffer == NULL) {
        printf("eroare alocare\n");
        exit(1);
    }

    memcpy(buffer, start, n);
    buffer[n] = '\0';
    return buffer;
}

//alex
int getNextAtom() {
    int state = 0;
    int nCh;
    char ch;
    Atom* at;
    char* value;

    for (;;) {
        ch = *pch;

        switch (state) {
        case 0:
            if (ch == ',') {
                state = 1;
                pch++;
            }
            else if (ch == ';') {
                state = 2;
                pch++;
            }
            else if (ch == '(') {
                state = 3;
                pch++;
            }
            else if (ch == ')') {
                state = 4;
                pch++;
            }
            else if (ch == '[') {
                state = 5;
                pch++;
            }
            else if (ch == ']') {
                state = 6;
                pch++;
            }
            else if (ch == '{') {
                state = 7;
                pch++;
            }
            else if (ch == '}') {
                state = 8;
                pch++;
            }
            else if (ch == '\0') { //EOF
                state = 9;
                pch++;
            }
            else if (ch == '&') {
                state = 10;
                pch++;
            }
            else if (ch == '|') {
                state = 12;
                pch++;
            }
            else if (ch == '!') {
                state = 14;
                pch++;
            }
            else if (ch == '=') {
                state = 17;
                pch++;
            }
            else if (ch == '<') {
                state = 20;
                pch++;
            }
            else if (ch == '>') {
                state = 23;
                pch++;
            }
            else if (ch == '+') {
                state = 26;
                pch++;
            }
            else if (ch == '-') {
                state = 27;
                pch++;
            }
            else if (ch == '*') {
                state = 28;
                pch++;
            }
            else if (ch == '/') {
                state = 29;
                pch++;
            }
            else if (ch == '.') {
                state = 31;
                pch++;
            }
            else if (isalpha(ch) || ch == '_') {
                state = 33;
                pStartCh = pch;
                pch++;
            }
            else if (ch == '\"') {
                state = 35;
                pStartCh = pch;
                pch++;
            }
            else if (ch == '\'') {
                state = 37;
                pStartCh = pch;
                pch++;
            }
            else if (ch >= '0' && ch <= '9') {
                state = 40;
                pStartCh = pch;
                pch++;
            }
            else if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') {
                state = 0;
                pch++;
                if (ch == '\n') {
                    line++;
                }
            }
            else aterr(addAtom(END), "caracter invalid\n");
            break;

        case 1:
            addAtom(COMMA);
            return COMMA;
        case 2:
            addAtom(SEMICOLON);
            return SEMICOLON;
        case 3:
            addAtom(LPAR);
            return LPAR;
        case 4:
            addAtom(RPAR);
            return RPAR;
        case 5:
            addAtom(LBRACKET);
            return LBRACKET;
        case 6:
            addAtom(RBRACKET);
            return RBRACKET;
        case 7:
            addAtom(LACC);
            return LACC;
        case 8:
            addAtom(RACC);
            return RACC;
        case 9:
            addAtom(END);
            return END;
        case 10:
            if (ch == '&') {
                state = 11;
                pch++;
            }
            else err("caracter invalid dupa &\n");
            break;
        case 11:
            addAtom(AND);
            return AND;
        case 12:
            if (ch == '|') {
                state = 13;
                pch++;
            }
            else err("caracter invalid dupa |\n");
            break;
        case 13:
            addAtom(OR);
            return OR;
        case 14:
            if (ch == '=') {
                state = 15;
                pch++;
            }
            else {
                state = 16;
            }
            break;
        case 15:
            addAtom(NOTEQ);
            return NOTEQ;
        case 16:
            addAtom(NOT);
            return NOT;
        case 17:
            if (ch == '=') {
                state = 18;
                pch++;
            }
            else {
                state = 19;
            }
            break;
        case 18:
            addAtom(EQUAL);
            return EQUAL;
        case 19:
            addAtom(ASSIGN);
            return ASSIGN;
        case 20:
            if (ch == '=') {
                state = 21;
                pch++;
            }
            else {
                state = 22;
            }
            break;
        case 21:
            addAtom(LESSEQ);
            return LESSEQ;
        case 22:
            addAtom(LESS);
            return LESS;
        case 23:
            if (ch == '=') {
                state = 24;
                pch++;
            }
            else {
                state = 25;
            }
            break;
        case 24:
            addAtom(GREATEREQ);
            return GREATEREQ;
        case 25:
            addAtom(GREATER);
            return GREATER;
        case 26:
            addAtom(ADD);
            return ADD;
        case 27:
            addAtom(SUB);
            return SUB;
        case 28:
            addAtom(MUL);
            return MUL;
        case 29:
            if (ch == '/') {
                state = 32;
                pch++;
            }
            else {
                state = 30;
            }
            break;
        case 30:
            addAtom(DIV);
            return DIV;
        case 31:
            addAtom(DOT);
            return DOT;
        case 32:
            if (ch != '\n' && ch != '\r' && ch != '\0') {
                state = 32;
                pch++;
            }
            else {
                state = 0;
            }
            break;
        case 33:
            if (isalnum(ch) || ch == '_') {
                state = 33;
                pch++;
            }
            else {
                state = 34;
            }
            break;
        case 34:
            nCh = pch - pStartCh;
            if (nCh == 5 && !memcmp(pStartCh, "break", 5)) at = addAtom(BREAK);
            else if (nCh == 4 && !memcmp(pStartCh, "char", 4)) at = addAtom(CHAR);
            else if (nCh == 6 && !memcmp(pStartCh, "double", 6)) at = addAtom(DOUBLE);
            else if (nCh == 4 && !memcmp(pStartCh, "else", 4)) at = addAtom(ELSE);
            else if (nCh == 3 && !memcmp(pStartCh, "for", 3)) at = addAtom(FOR);
            else if (nCh == 2 && !memcmp(pStartCh, "if", 2)) at = addAtom(IF);
            else if (nCh == 3 && !memcmp(pStartCh, "int", 3)) at = addAtom(INT);
            else if (nCh == 6 && !memcmp(pStartCh, "return", 6)) at = addAtom(RETURN);
            else if (nCh == 6 && !memcmp(pStartCh, "struct", 6)) at = addAtom(STRUCT);
            else if (nCh == 4 && !memcmp(pStartCh, "void", 4)) at = addAtom(VOID);
            else if (nCh == 5 && !memcmp(pStartCh, "while", 5)) at = addAtom(WHILE);
            else {
                at = addAtom(ID);
                at->text = createString(pStartCh, pch);
            }
            return at->code;
        case 35:
            if (ch != '\"') {
                state = 35;
                pch++;
            }
            else if (ch == '\"') {
                state = 36;
                pch++;
            }
            break;
        case 36:
            at = addAtom(CT_STRING);
            at->text = createString(pStartCh + 1, pch - 1);
            return CT_STRING;
        case 37:
            if (ch != '\'') {
                state = 38;
                pch++;
            }
            break;
        case 38:
            if (ch == '\'') {
                state = 39;
                pch++;
            }
            break;
        case 39:
            at = addAtom(CT_CHAR);
            value = createString(pStartCh + 1, pch - 1);
            at->i = *value;
            return CT_CHAR;
        case 40:
            if (ch >= '0' && ch <= '9') {
                state = 40;
                pch++;
            }
            else if (ch == '.') {
                state = 42;
                pch++;
            }
            else if (ch == 'e' || ch == 'E') {
                state = 44;
                pch++;
            }
            else {
                state = 41;
            }
            break;
        case 41:
            at = addAtom(CT_INT);
            value = createString(pStartCh, pch);
            at->i = strtol(value, NULL, 10);  //transforma value din string in numar in baza 10
            return CT_INT;
        case 42:
            if (isdigit(ch)) {
                state = 43;
                pch++;
            }
            else if (ch == 'e' || ch == 'E') {
                state = 48;
                pch++;
            }
            else err("dupa punct trebuie sa urmeze ceva\n");
            break;
        case 43:
            if (isdigit(ch)) {
                state = 43;
                pch++;
            }
            else if (ch == 'e' || ch == 'E') {
                state = 44;
                pch++;
            }
            else {
                state = 47;
            }
            break;
        case 44:
            if (ch == '-' || ch == '+') {
                state = 45;
                pch++;
            }
            else {
                state = 45;
            }
            break;
        case 45:
            if (isdigit(ch)) {
                state = 46;
                pch++;
            }
            break;
        case 46:
            if (isdigit(ch)) {
                state = 46;
                pch++;
            }
            else {
                state = 47;
            }
            break;
        case 47:
            at = addAtom(CT_REAL);
            value = createString(pStartCh, pch);
            at->r = atof(value);    //transforma stringul intr-o valoare reala
            return CT_REAL;
        default:
            addAtom(END);
            return END;
        }
    }
}

const char* atomNames[] = { "ID", "CT_INT", "CT_REAL", "CT_CHAR", "CT_STRING",
      "BREAK", "CHAR", "DOUBLE", "ELSE", "FOR", "IF", "INT", "RETURN", "STRUCT", "VOID", "WHILE",
      "COMMA", "SEMICOLON", "LPAR", "RPAR", "LBRACKET", "RBRACKET", "LACC", "RACC", "END",
      "ADD", "SUB", "MUL", "DIV", "DOT", "AND", "OR", "NOT", "ASSIGN", "EQUAL", "NOTEQ", "LESS", "LESSEQ", "GREATER", "GREATEREQ"
};

void showAtom(Atom* at) {
    printf("%d %s", at->line, atomNames[at->code]);
    if (at->code == ID) {
        printf(":%s ", at->text);
    }
    else if (at->code == CT_INT) {
        printf(":%d ", at->i);
    }
    else if (at->code == CT_REAL) {
        printf(":%lf ", at->r);
    }
    else if (at->code == CT_CHAR) {
        printf(":%c ", (char)at->i);
    }
    else if (at->code == CT_STRING) {
        printf(":%s ", at->text);
    }
}

void showAtoms() {
    Atom* aux = atoms;
    while (aux != NULL) {
        showAtom(aux);
        printf("\n");
        aux = aux->next;
    }
}

//asin

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

typedef enum {		// felul unui simbol
    SK_VAR, SK_PARAM, SK_FN, SK_STRUCT
}SymKind;

struct Symbol {
    const char* name;
    SymKind kind;
    Type type;
    Symbol* owner;
    Symbol* next;
    union {
        int varIdx;
        int paramIdx;
        Symbol* structMembers;
        struct {
            Symbol* params;
            Symbol* locals;
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

typedef struct {
    Type type;		
    bool lval;			
    bool ct;				
}Ret;

bool canBeScalar(Ret* r);
bool convTo(Type* src, Type* dst);
bool arithTypeTo(Type* t1, Type* t2, Type* dst);
Symbol* findSymbolInList(Symbol* list, const char* name);

bool canBeScalar(Ret* r) {
    Type* t = &r->type;
    if (t->n >= 0)return false;
    if (t->tb == TB_VOID)return false;
    return true;
}

bool convTo(Type* src, Type* dst) {
    
    if (src->n >= 0) {
        if (dst->n >= 0)return true;
        return false;
    }
    if (dst->n >= 0)return false;
    switch (src->tb) {
    case TB_INT:
    case TB_DOUBLE:
    case TB_CHAR:
        switch (dst->tb) {
        case TB_INT:
        case TB_CHAR:
        case TB_DOUBLE:
            return true;
        default:return false;
        }
        
    case TB_STRUCT:
        if (dst->tb == TB_STRUCT && src->s == dst->s)return true;
        return false;
    default:return false;
    }
}

bool arithTypeTo(Type* t1, Type* t2, Type* dst) {
    
    if (t1->n >= 0 || t2->n >= 0)return false;
  
    dst->s = NULL;
    dst->n = -1;
    switch (t1->tb) {
    case TB_INT:
        switch (t2->tb) {
        case TB_INT:
        case TB_CHAR:
            dst->tb = TB_INT; return true;
        case TB_DOUBLE:dst->tb = TB_DOUBLE; return true;
        default:return false;
        }
    case TB_DOUBLE:
        switch (t2->tb) {
        case TB_INT:
        case TB_DOUBLE:
        case TB_CHAR:
            dst->tb = TB_DOUBLE; return true;
        default:return false;
        }
    case TB_CHAR:
        switch (t2->tb) {
        case TB_INT:
        case TB_DOUBLE:
        case TB_CHAR:
            dst->tb = t2->tb; return true;
        default:return false;
        }
    default:return false;
    }
}

Symbol* findSymbolInList(Symbol* list, const char* name) {
    for (Symbol* s = list; s; s = s->next) {
        if (!strcmp(s->name, name))return s;
    }
    return NULL;
}

int typeBaseSize(Type* t) {
    switch (t->tb) {
    case TB_INT:return sizeof(int);
    case TB_DOUBLE:return sizeof(double);
    case TB_CHAR:return sizeof(char);
    case TB_VOID:return 0;
    default: {
        int size = 0;
        for (Symbol* m = t->s->structMembers; m; m = m->next) {
            size += typeSize(&m->type);
        }
        return size;
    }
    }
}

int typeSize(Type* t) {
    if (t->n < 0)return typeBaseSize(t);
    if (t->n == 0)return sizeof(void*);
    return t->n * typeBaseSize(t);
}

void freeSymbols(Symbol* list) {
    for (Symbol* next; list; list = next) {
        next = list->next;
        freeSymbol(list);
    }
}

Symbol* newSymbol(const char* name, SymKind kind) {
    Symbol* s;
    SAFEALLOC(s, Symbol);
    memset(s, 0, sizeof(Symbol));
    s->name = name;
    s->kind = kind;
    return s;
}

Symbol* dupSymbol(Symbol* symbol) {
    Symbol* s;
    SAFEALLOC(s, Symbol);
    *s = *symbol;
    s->next = NULL;
    return s;
}

Symbol* addSymbolToList(Symbol** list, Symbol* s) {
    Symbol* iter = *list;
    if (iter) {
        while (iter->next)iter = iter->next;
        iter->next = s;
    }
    else {
        *list = s;
    }
    return s;
}

int symbolsLen(Symbol* list) {
    int n = 0;
    for (; list; list = list->next)n++;
    return n;
}

void freeSymbol(Symbol* s) {
    switch (s->kind) {
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

Domain* pushDomain() {
    Domain* d;
    SAFEALLOC(d, Domain);
    d->symbols = NULL;
    d->parent = symTable;
    symTable = d;
    return d;
}

void dropDomain() {
    Domain* d = symTable;
    symTable = d->parent;
    freeSymbols(d->symbols);
    free(d);
}

void showNamedType(Type* t, const char* name) {
    switch (t->tb) {
    case TB_INT:printf("int"); break;
    case TB_DOUBLE:printf("double"); break;
    case TB_CHAR:printf("char"); break;
    case TB_VOID:printf("void"); break;
    default:
        printf("struct %s", t->s->name);
    }
    if (name)printf(" %s", name);
    if (t->n == 0)printf("[]");
    else if (t->n > 0)printf("[%d]", t->n);
}

void showSymbol(Symbol* s) {
    switch (s->kind) {
    case SK_VAR:
        showNamedType(&s->type, s->name);
        printf(";\t// size=%d, idx=%d\n", typeSize(&s->type), s->varIdx);
        break;
    case SK_PARAM: {
        showNamedType(&s->type, s->name);
        printf(" /*size=%d, idx=%d*/", typeSize(&s->type), s->paramIdx);
    }break;
    case SK_FN: {
        showNamedType(&s->type, s->name);
        printf("(");
        bool next = false;
        for (Symbol* param = s->fn.params; param; param = param->next) {
            if (next)printf(", ");
            showSymbol(param);
            next = true;
        }
        printf("){\n");
        for (Symbol* local = s->fn.locals; local; local = local->next) {
            printf("\t");
            showSymbol(local);
        }
        printf("\t}\n");
    }break;
    case SK_STRUCT: {
        printf("struct %s{\n", s->name);
        for (Symbol* m = s->structMembers; m; m = m->next) {
            printf("\t");
            showSymbol(m);
        }
        printf("\t};\t// size=%d\n", typeSize(&s->type));
    }break;
    }
}

void showDomain(Domain* d, const char* name) {
    printf("// domain: %s\n", name);
    for (Symbol* s = d->symbols; s; s = s->next) {
        showSymbol(s);
    }
    puts("\n");
}

Symbol* findSymbolInDomain(Domain* d, const char* name) {
    for (Symbol* s = d->symbols; s; s = s->next) {
        if (!strcmp(s->name, name))return s;
    }
    return NULL;
}

Symbol* findSymbol(const char* name) {
    for (Domain* d = symTable; d; d = d->parent) {
        Symbol* s = findSymbolInDomain(d, name);
        if (s)return s;
    }
    return NULL;
}

Symbol* addSymbolToDomain(Domain* d, Symbol* s) {
    return addSymbolToList(&d->symbols, s);
}

int allocInGlobalMemory(int nBytes) {
    char* aux = (char*)realloc(globalMemory, nGlobalMemory + nBytes);
    if (!aux)err("not enough memory");
    int idx = nGlobalMemory;
    nGlobalMemory += nBytes;
    return idx;
}

bool unit();
bool structDef();
bool varDef();
bool typeBase(Type* t);
bool arrayDecl(Type* t);
bool fnDef();
bool fnParam();
bool stm();
bool stmCompound(bool newDomain);
bool expr(Ret *r);
bool exprAssign(Ret* r);
bool exprOr(Ret* r);
bool exprAnd(Ret* r);
bool exprEq(Ret* r);
bool exprRel(Ret* r);
bool exprAdd(Ret* r);
bool exprMul(Ret* r);
bool exprCast(Ret* r);
bool exprUnary(Ret* r);
bool exprPostfix(Ret* r);
bool exprPrimary(Ret* r);

Atom* iAt;
Atom* consumedAt;

bool consume(int code) {
    if (iAt->code == code) {
        consumedAt = iAt;
        iAt = iAt->next;
        printf("consume(%s)", atomNames[code]);
        printf(" => consumed\n");
        return true;
    }
    return false;
}

// typeBase: INT | DOUBLE | CHAR | STRUCT ID
bool typeBase(Type* t) {
    Atom* start = iAt;
    t->n = -1;

    if (consume(INT)) {
        t->tb = TB_INT;
        return true;
    }
    if (consume(DOUBLE)) {
        t->tb = TB_DOUBLE;
        return true;
    }
    if (consume(CHAR)) {
        t->tb = TB_CHAR;
        return true;
    }
    if (consume(STRUCT)) {
        if (consume(ID)) {
            Atom* atName = consumedAt;
            t->tb = TB_STRUCT;
            t->s = findSymbol(atName->text);
            if (!t->s) aterr(iAt, "structura nedefinita: %s\n", atName->text);
            return true;
        }
        else aterr(iAt, "lipseste numele structurii\n");
    }

    iAt = start;
    return false;
}


// exprPostfixPrim: LBRACKET expr RBRACKET exprPostfixPrim | DOT ID exprPostfixPrim | eps
bool exprPostfixPrim(Ret *r) {
    Atom* start = iAt;

    if (consume(LBRACKET)) {
        Ret idx;
        if (expr(&idx)) {
            if (consume(RBRACKET)) {
                if (r->type.n < 0) aterr(iAt, "only an array can be indexed");
                Type tInt = { TB_INT,NULL,-1 };
                if (!convTo(&idx.type, &tInt)) aterr(iAt, "the index is not convertible to int");
                r->type.n = -1;
                r->lval = true;
                r->ct = false;
                if (exprPostfixPrim(r)) {
                    return true;
                }
                else aterr(iAt, "expresie invalida dupa ]\n");
            }
            else aterr(iAt, "lipseste ]n");
        }
        else aterr(iAt, "lipseste expresia de dupa [\n");
    }
    if (consume(DOT)) {
        if (consume(ID)) {
            Atom* atName = consumedAt;
            if (r->type.tb != TB_STRUCT) aterr(iAt, "a field can only be selected from a struct");
            Symbol* s = findSymbolInList(r->type.s->structMembers, atName->text);
            if (!s) aterr(iAt, "the structure %s does not have a field %s",r->type.s->name,atName->text);
            * r = (Ret){ s->type,true,s->type.n >= 0 };
            if (exprPostfixPrim(r)) {
                return true;
            }
            else aterr(iAt, "expresie invalida dupa ID\n");
        }
        else aterr(iAt, "lipseste ID dupa . \n");
    }

    iAt = start;
    return true;
}

// exprPostfix: exprPrimary exprPostfixPrim
bool exprPostfix(Ret *r) {
    Atom* start = iAt;

    if (exprPrimary(r)) {
        if (exprPostfixPrim(r)) {
            return true;
        }
        else aterr(iAt, "expresie invalida in exprPostfix\n");
    }

    iAt = start;
    return false;
}

// exprUnary: ( SUB | NOT ) exprUnary | exprPostfix
bool exprUnary(Ret *r) {
    Atom* start = iAt;

    if (consume(SUB)) {
        if (exprUnary(r)) {
            if (!canBeScalar(r)) aterr(iAt, "unary - must have a scalar operand");
            r->lval = false;
            r->ct = true;
            return true;
        }
        else aterr(iAt, "expresie invalida dupa - \n");
    }
    else if (consume(NOT)) {
        if (exprUnary(r)) {
            if (!canBeScalar(r)) aterr(iAt, "unary - must have a scalar operand");
            r->lval = false;
            r->ct = true;
            return true;
        }
        else aterr(iAt, "expresie invalida dupa ! \n");
    }

    if (exprPostfix(r)) {
        return true;
    }

    iAt = start;
    return false;
}

// exprCast: LPAR typeBase arrayDecl? RPAR exprCast | exprUnary
bool exprCast(Ret *r) {
    Atom* start = iAt;
    //Type t;

    if (consume(LPAR)) {
        Type t;
        Ret op;
        if (typeBase(&t)) {
            if (arrayDecl(&t)) {}
            if (consume(RPAR)) {
                if (exprCast(&op)) {
                    if (t.tb == TB_STRUCT) aterr(iAt, "cannot convert to a struct type");
                    if (op.type.tb == TB_STRUCT) aterr(iAt, "cannot convert a struct");
                    if (op.type.n >= 0 && t.n < 0) aterr(iAt, "an array can be converted only to another array");
                    if (op.type.n < 0 && t.n >= 0) aterr(iAt, "a scalar can be converted only to another scalar");
                    *r = (Ret){ t,false,true };
                    return true;
                }
                else aterr(iAt, "expresie invalida dupa ) in expresia de cast\n");
            }
            else aterr(iAt, "lipseste ) din exprresia de cast\n");
        }
    }

    if (exprUnary(r)) {
        return true;
    }

    iAt = start;
    return false;
}


// exprMulPrim: (MUL | DIV) exprCast exprMulPrim | eps
bool exprMulPrim(Ret *r) {
    Atom* start = iAt;

    if (consume(MUL)) {
        Ret right;
        if (exprCast(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) aterr(iAt, "inavlid operand type for *\n");
            *r = (Ret){ tDst, false, true };
            if (exprMulPrim(r)) {
                return true;
            }
            else aterr(iAt, "expresie invalida dupa cast\n");
        }
        else aterr(iAt, "expresie invalida dupa * \n");
    }
    else if (consume(DIV)) {
        Ret right;
        if (exprCast(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) aterr(iAt, "inavlid operand type for /\n");
            *r = (Ret){ tDst, false, true };
            if (exprMulPrim(r)) {
                return true;
            }
            else aterr(iAt, "expresie invalida dupa cast\n");
        }
        else aterr(iAt, "expresie invalida dupa / \n");
    }

    iAt = start;
    return true;
}

// exprMul: exprCast exprMulPrim
bool exprMul(Ret *r) {
    Atom* start = iAt;

    if (exprCast(r)) {
        if (exprMulPrim(r)) {
            return true;
        }
        else aterr(iAt, "expresie invalida dupa cast\n");
    }

    iAt = start;
    return false;
}

// exprAddPrim: (ADD | SUB) exprMul exprAddPrim | eps
bool exprAddPrim(Ret *r) {
    Atom* start = iAt;

    if (consume(ADD)) {
        Ret right;
        if (exprMul(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) aterr(iAt, "inavlid operand type for +\n");
            *r = (Ret){ tDst, false, true };
            if (exprAddPrim(r)) {
                return true;
            }
            else aterr(iAt, "expresie invalida dupa inmultire\n");
        }
        else aterr(iAt, "expresie invalida dupa + \n");
    }
    else if (consume(SUB)) {
        Ret right;
        if (exprMul(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) aterr(iAt, "inavlid operand type for -\n");
            *r = (Ret){ tDst, false, true };
            if (exprAddPrim(r)) {
                return true;
            }
            else aterr(iAt, "expresie invalida dupa inmultire \n");
        }
        else aterr(iAt, "expresie invalida dupa - \n");
    }

    iAt = start;
    return true;
}

// exprAdd: exprMul exprAddPrim
bool exprAdd(Ret *r) {
    Atom* start = iAt;

    if (exprMul(r)) {
        if (exprAddPrim(r)) {
            return true;
        }
        else aterr(iAt, "expresie invalida dupa inmultire \n");
    }

    iAt = start;
    return false;
}

// exprRelPrim: (LESS | LESSEQ | GREATER | GREATEREQ) exprAdd exprRelPrim | eps
bool exprRelPrim(Ret *r) {
    Atom* start = iAt;

    if (consume(LESS)) {
        Ret right;
        if (exprAdd(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) aterr(iAt, "inavlid operand type for <\n");
            *r = (Ret){ {TB_INT, NULL, -1}, false, true };
            if (exprRelPrim(r)) {
                return true;
            }
            else aterr(iAt, "expresie invalida dupa adunare\n");
        }
        else aterr(iAt, "expresie invalida dupa < \n");
    }
    else if (consume(LESSEQ)) {
        Ret right;
        if (exprAdd(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) aterr(iAt, "inavlid operand type for <=\n");
            *r = (Ret){ {TB_INT, NULL, -1}, false, true };
            if (exprRelPrim(r)) {
                return true;
            }
            else aterr(iAt, "expresie invalida dupa adunare \n");
        }
        else aterr(iAt, "expresie invalida dupa <= \n");
    }
    else if (consume(GREATER)) {
        Ret right;
        if (exprAdd(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) aterr(iAt, "inavlid operand type for >\n");
            *r = (Ret){ {TB_INT, NULL, -1}, false, true };
            if (exprRelPrim(r)) {
                return true;
            }
            else aterr(iAt, "expresie invalida dupa adunare \n");
        }
        else aterr(iAt, "expresie invalida dupa > \n");
    }
    else if (consume(GREATEREQ)) {
        Ret right;
        if (exprAdd(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) aterr(iAt, "inavlid operand type for >=\n");
            *r = (Ret){ {TB_INT, NULL, -1}, false, true };
            if (exprRelPrim(r)) {
                return true;
            }
            else aterr(iAt, "expresie invalida dupa adunare \n");
        }
        else aterr(iAt, "expresie invalida dupa >= \n");
    }

    iAt = start;
    return true;
}

// exprRel: exprAdd exprRelPrim
bool exprRel(Ret *r) {
    Atom* start = iAt;

    if (exprAdd(r)) {
        if (exprRelPrim(r)) {
            return true;
        }
        else aterr(iAt, "expresie invalida dupa adunare\n");
    }

    iAt = start;
    return false;
}

// exprEqPrim: (EQUAL | NOTEQ) exprRel exprEqPrim | eps
bool exprEqPrim(Ret *r) {
    Atom* start = iAt;

    if (consume(EQUAL)) {
        Ret right;
        if (exprRel(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) aterr(iAt, "inavlid operand type for ==\n");
            *r = (Ret){ {TB_INT, NULL, -1}, false, true };
            if (exprEqPrim(r)) {
                return true;
            }
            else aterr(iAt, "expresie invalida dupa expresia de relationare\n");
        }
        else aterr(iAt, "expresie invalida dupa == \n");
    }
    else if (consume(NOTEQ)) {
        Ret right;
        if (exprRel(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) aterr(iAt, "inavlid operand type for !=\n");
            *r = (Ret){ {TB_INT, NULL, -1}, false, true };
            if (exprEqPrim(r)) {
                return true;
            }
            else aterr(iAt, "expresie invalida dupa expresia de relationare \n");
        }
        else aterr(iAt, "expresie invalida dupa != \n");
    }

    iAt = start;
    return true;
}

// exprEq: exprRel exprEqPrim
bool exprEq(Ret *r) {
    Atom* start = iAt;

    if (exprRel(r)) {
        if (exprEqPrim(r)) {
            return true;
        }
        else aterr(iAt, "expresie invalida dupa expresia de relationare \n");
    }

    iAt = start;
    return false;
}

// exprAndPrim: AND exprEq exprAndPrim | eps
bool exprAndPrim(Ret *r) {
    Atom* start = iAt;

    if (consume(AND)) {
        Ret right;
        if (exprEq(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) aterr(iAt, "inavlid operand type for &&\n");
            *r = (Ret){ {TB_INT, NULL, -1}, false, true };
            if (exprAndPrim(r)) {
                return true;
            }
            else aterr(iAt, "expresie invalida dupa egalitate\n");
        }
        else aterr(iAt, "expresie invalida dupa && \n");
    }

    iAt = start;
    return true;
}

// exprAnd: exprEq exprAndPrim
bool exprAnd(Ret *r) {
    Atom* start = iAt;

    if (exprEq(r)) {
        if (exprAndPrim(r)) {
            return true;
        }
        else aterr(iAt, "expresie invalida dupa egalitate\n");
    }

    iAt = start;
    return false;
}

// exprOrPrim: OR exprAnd exprOrPrim | eps
bool exprOrPrim(Ret *r) {
    Atom* start = iAt;

    if (consume(OR)) {
        Ret right;
        if (exprAnd(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) aterr(iAt, "inavlid operand type for ||\n");
            *r = (Ret){ {TB_INT, NULL, -1}, false, true };
            if (exprOrPrim(r)) {
                return true;
            }
            else aterr(iAt, "expresie invalida dupa && \n");
        }
        else aterr(iAt, "expresie invalida dupa || \n");
    }

    iAt = start;
    return true;
}

// exprOr: exprAnd exprOrPrim
bool exprOr(Ret *r) {
    Atom* start = iAt;

    if (exprAnd(r)) {
        if (exprOrPrim(r)) {
            return true;
        }
        else aterr(iAt, "expresie invalida dupa && \n");
    }

    iAt = start;
    return false;
}

// exprAssign: exprUnary ASSIGN exprAssign | exprOr
bool exprAssign(Ret *r) {
    Atom* start = iAt;
    Ret rDst;

    if (exprUnary(&rDst)) {
        if (consume(ASSIGN)) {
            if (exprAssign(r)) {
                if (!rDst.lval) aterr(iAt, "the assign destination must be a left-value\n");
                if (rDst.ct) aterr(iAt, "the assign destination cannot be constant");
                if (!canBeScalar(&rDst)) aterr(iAt, "the assign destination must be scalar");
                if (!canBeScalar(r)) aterr(iAt, "the assign source must be scalar");
                if (!convTo(&r->type, &rDst.type)) aterr(iAt, "the assign source cannot be converted to destination");
                r->lval = false;
                r->ct = true;
                return true;
            }
            else aterr(iAt, "expresie invalida dupa = \n");
        }
    }

    iAt = start;
    if (exprOr(r)) {
        return true;
    }

    iAt = start;
    return false;
}

// expr: exprAssign
bool expr(Ret *r) {
    Atom* start = iAt;

    if (exprAssign(r)) {
        return true;
    }

    iAt = start;
    return false;
}

// arrayDecl: LBRACKET CT_INT? RBRACKET
bool arrayDecl(Type* t) {
    Atom* start = iAt;
    if (consume(LBRACKET)) {
        if (consume(CT_INT)) {
            Atom* atSize = consumedAt;
            t->n = atSize->i;
        }
        else {
            t->n = 0;
        }
        if (consume(RBRACKET)) {
            return true;
        }
        else aterr(iAt, "lipseste ] din declararea array-ului\n");
    }

    iAt = start;
    return false;
}

// varDef: typeBase ID arrayDecl? SEMICOLON
bool varDef() {
    Atom* start = iAt;
    Type t;

    if (typeBase(&t)) {
        if (consume(ID)) {
            Atom* atName = consumedAt;
            if (arrayDecl(&t)) {
                if (t.n == 0) aterr(iAt, "a vector variable must have a specified dimension\n");
            }
            if (consume(SEMICOLON)) {

                Symbol* var = findSymbolInDomain(symTable, atName->text);
                if (var) aterr(iAt, "symbol redefinition: %s\n", atName->text);
                var = newSymbol(atName->text, SK_VAR);
                var->type = t;
                var->owner = owner;
                addSymbolToDomain(symTable, var);
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
            }
            else aterr(iAt, "lipseste ; dupa definitia variabilei\n");
        }
        else aterr(iAt, "lipseste numele variabilei\n");
    }

    iAt = start;
    return false;
}

// structDef: STRUCT ID LACC varDef* RACC SEMICOLON
bool structDef() {
    Atom* start = iAt;

    if (consume(STRUCT)) {
        if (consume(ID)) {
            Atom* atName = consumedAt;
            if (consume(LACC)) {

                Symbol* s = findSymbolInDomain(symTable, atName->text);
                if (s) aterr(iAt, "symbol redefinition: %s\n", atName->text);
                s = addSymbolToDomain(symTable, newSymbol(atName->text, SK_STRUCT));
                s->type.tb = TB_STRUCT;
                s->type.s = s;
                s->type.n = -1;
                pushDomain();
                owner = s;

                for (;;) {
                    if (varDef()) {}
                    else break;
                }
                if (consume(RACC)) {
                    if (consume(SEMICOLON)) {
                        owner = NULL;
                        dropDomain();
                        return true;
                    }
                    else aterr(iAt, "lipseste ; dupa definitia structurii\n");
                }
                else aterr(iAt, "lipseste } din definitia structurii\n");
            }
        }
        else aterr(iAt, "lipseste numele structurii\n");
    }

    iAt = start;
    return false;
}

// fnParam: typeBase ID arrayDecl?
bool fnParam() {
    Atom* start = iAt;
    Type t;

    if (typeBase(&t)) {
        if (consume(ID)) {
            Atom* atName = consumedAt;
            if (arrayDecl(&t)) {
                //t.n = 0;
            }

            Symbol* param = findSymbolInDomain(symTable, atName->text);
            if (param) aterr(iAt, "symbol redefinition: %s\n", atName->text);
            param = newSymbol(atName->text, SK_PARAM);
            param->type = t;
            param->paramIdx = symbolsLen(owner->fn.params);
            addSymbolToDomain(symTable, param);
            addSymbolToList(&owner->fn.params, dupSymbol(param));

            return true;
        }
        else aterr(iAt, "lipseste numele parametrului functiei\n");
    }

    iAt = start;
    return false;
}

// fnDef: ( typeBase | VOID ) ID
//        LPAR(fnParam(COMMA fnParam)*) ? RPAR
//        stmCompound
bool fnDef() {
    Atom* start = iAt;
    Type t;

    if (typeBase(&t)) {
        if (consume(ID)) {
            Atom* atName = consumedAt;
            if (consume(LPAR)) {

                Symbol* fn = findSymbolInDomain(symTable, atName->text);
                if (fn) aterr(iAt, "symbol redefinition: %s\n", atName->text);
                fn = newSymbol(atName->text, SK_FN);
                fn->type = t;
                addSymbolToDomain(symTable, fn);
                owner = fn;
                pushDomain();

                if (fnParam()) {
                    for (;;) {
                        if (consume(COMMA)) {
                            if (fnParam()) {}
                            else aterr(iAt, "lipseste argumentul dupa , din definitia functiei\n");
                        }
                        else break;
                    }
                }
                if (consume(RPAR)) {
                    if (stmCompound(false)) {
                        dropDomain();
                        owner = NULL;

                        return true;
                    }
                    else aterr(iAt, "lipseste corpul functiei\n");
                }
                else aterr(iAt, "lipseste ) dupa argumentele functiei\n");
            }

        }
        else aterr(iAt, "lipseste numele structurii\n");
    }
    else if (consume(VOID)) {
        t.tb = TB_VOID;
        if (consume(ID)) {
            Atom* atName = consumedAt;
            if (consume(LPAR)) {

                Symbol* fn = findSymbolInDomain(symTable, atName->text);
                if (fn) aterr(iAt, "symbol redefinition: %s\n", atName->text);
                fn = newSymbol(atName->text, SK_FN);
                fn->type = t;
                addSymbolToDomain(symTable, fn);
                owner = fn;
                pushDomain();

                if (fnParam()) {
                    for (;;) {
                        if (consume(COMMA)) {
                            if (fnParam()) {}
                            else aterr(iAt, "lipseste argumentul dupa , \n");
                        }
                        else break;
                    }
                }
                if (consume(RPAR)) {
                    if (stmCompound(false)) {
                        dropDomain();
                        owner = NULL;

                        return true;
                    }
                    else aterr(iAt, "lipseste corpul functiei\n");
                }
                else aterr(iAt, "lipseste ) dupa argumentele functiei\n");
            }

        }
        else aterr(iAt, "lipseste numele structurii\n");

    }

    iAt = start;
    return false;
}

// unit: ( structDef | fnDef | varDef )* END
bool unit() {
    Atom* start = iAt;

    for (;;) {
        if (structDef()) {}
        else if (fnDef()) {}
        else if (varDef()) {}
        else break;
    }
    if (consume(END)) return true;
    aterr(iAt, "eroare sintaxa\n");
    iAt = start;
    return false;
}

// stmCompound: LACC ( varDef | stm )* RACC
bool stmCompound(bool newDomain) {
    Atom* start = iAt;

    if (consume(LACC)) {
        if (newDomain) pushDomain();
        for (;;) {
            if (varDef()) {}
            else if (stm()) {}
            else break;
        }
        if (consume(RACC)) {
            if (newDomain) dropDomain();
            return true;
        }
        else aterr(iAt, "lipseste } dupa definirea corpului functiei\n");
    }

    iAt = start;
    return false;
}

// stm: stmCompound
//      | IF LPAR expr RPAR stm(ELSE stm) ?
//      | WHILE LPAR expr RPAR stm
//      | FOR LPAR expr ? SEMICOLON expr ? SEMICOLON expr ? RPAR stm
//      | BREAK SEMICOLON
//      | RETURN expr ? SEMICOLON
//      | expr ? SEMICOLON
bool stm() {
    Atom* start = iAt;
    Ret rInit, rCond, rStep, rExpr;

    if (stmCompound(true)) {
        return true;
    }
    else if (consume(IF)) {
        if (consume(LPAR)) {
            if (expr(&rCond)) {
                if (!canBeScalar(&rCond)) aterr(iAt, "the if condition must be a scalar value\n");
                if (consume(RPAR)) {
                    if (stm()) {
                        if (consume(ELSE)) {
                            if (stm()) {
                                return 1;
                            }
                            else aterr(iAt, "lipseste corpul ramurii ELSE\n");
                        }
                        return 1;
                    }
                    else aterr(iAt, "lipseste corpul ramurii IF\n");
                }
                else aterr(iAt, "lipseste ) dupa conditia IF\n");
            }
            else aterr(iAt, "lipseste conditia din IF\n");
        }
        else aterr(iAt, "lipseste ( dupa IF\n");
    }
    else if (consume(WHILE)) {
        if (consume(LPAR)) {
            if (expr(&rCond)) {
                if (!canBeScalar(&rCond)) aterr(iAt, "the while condition must be a scalar value\n");
                if (consume(RPAR)) {
                    if (stm()) {
                        return true;
                    }
                    else aterr(iAt, "lipseste corpul lui WHILE\n");
                }
                else aterr(iAt, "lipseste ) dupa conditia din WHILE\n");
            }
            else aterr(iAt, "lipseste conditia din WHILE\n");
        }
        else aterr(iAt, "lipseste ( dupa WHILE\n");
    }
    else if (consume(FOR)) {
        if (consume(LPAR)) {
            if (expr(&rInit)) {}
            if (consume(SEMICOLON)) {
                if (expr(&rCond)) {
                    if (!canBeScalar(&rCond)) aterr(iAt, "the for condition must be a scalar value\n");
                }
                if (consume(SEMICOLON)) {
                    if (expr(&rStep)) {}
                    if (consume(RPAR)) {
                        if (stm()) {
                            return true;
                        }
                        else aterr(iAt, "lipseste corpul lui FOR\n");
                    }
                    else aterr(iAt, "lipseste ) dupa conditia din FOR\n");
                }
                else aterr(iAt, "lipseste al doilea ; din FOR\n");
            }
            else aterr(iAt, "lipseste primul ; din FOR\n");
        }
        else aterr(iAt, "lipseste ( dupa FOR\n");
    }
    else if (consume(BREAK)) {
        if (consume(SEMICOLON)) {
            return true;
        }
        else aterr(iAt, "lipseste ; dupa BREAK\n");
    }
    else if (consume(RETURN)) {
        if (expr(&rExpr)) {
            if (owner->type.tb == TB_VOID) aterr(iAt, "a void function cannot return value\n");
            if (!canBeScalar(&rExpr)) aterr(iAt, "the return value must be a scalar value\n");
            if (!convTo(&rExpr.type, &owner->type)) aterr(iAt, "cannot convert the return expression type to the function return type\n");
        }
        else if (owner->type.tb != TB_VOID) aterr(iAt, "a non-void function must return a value\n");

        if (consume(SEMICOLON)) {
            return true;
        }
        else aterr(iAt, "lipseste ; dupa RETURN\n");
    }
    else if (expr(&rExpr)) {
        if (consume(SEMICOLON)) {
            return true;
        }
        else aterr(iAt, "lipseste ; dupa expresia \n");
    }
    else if (consume(SEMICOLON)) {
        return true;
    }

    iAt = start;
    return false;
}

//exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
//             | CT_INT | CT_REAL | CT_CHAR | CT_STRING | LPAR expr RPAR
bool exprPrimary(Ret *r) {
    Atom* start = iAt;

    if (consume(ID)) {
        Atom* atName = consumedAt;
        Symbol* s = findSymbol(atName->text);
        if (!s) aterr(iAt, "undefined id: %s", atName->text);
        if (consume(LPAR)) {
            if (s->kind != SK_FN) aterr(iAt, "only a function can be called");
            Ret rArg;
            Symbol* param = s->fn.params;
            if (expr(&rArg)) {
                if (!param) aterr(iAt, "too many arguments in function call");
                if (!convTo(&rArg.type, &param->type)) aterr(iAt, "in call, cannot convert the argument type to the parameter type");
                param = param->next;
                for (;;) {
                    if (consume(COMMA)) {
                        if (expr(&rArg)) {
                            if (!param) aterr(iAt, "too many arguments in function call");
                            if (!convTo(&rArg.type, &param->type)) aterr(iAt, "in call, cannot convert the argument type to the parameter type");
                            param = param->next;
                        }
                        else aterr(iAt, "lipseste expresia dupa ,\n");
                    }
                    else break;
                }
            }
            if (consume(RPAR)) {
                if (param) aterr(iAt, "too few arguments in function call");
                *r = (Ret){ s->type,false,true };
                return true;
            }
            else aterr(iAt, "lipseste ) dupa expresia \n");
        }
        if (s->kind == SK_FN) aterr(iAt, "a function can only be called");
        *r = (Ret){ s->type,true,s->type.n >= 0 };

        return true;
    }

    if (consume(CT_INT)) {
        *r = (Ret){ {TB_INT,NULL,-1},false,true };
        return true;
    }

    if (consume(CT_REAL)) {
        *r = (Ret){ {TB_DOUBLE,NULL,-1},false,true };
        return true;
    }

    if (consume(CT_CHAR)) {
        *r = (Ret){ {TB_CHAR,NULL,-1},false,true };
        return true;
    }

    if (consume(CT_STRING)) {
        *r = (Ret){ {TB_CHAR,NULL,0},false,true };
        return true;
    }

    if (consume(LPAR)) {
        if (expr(r)) {
            if (consume(RPAR)) {
                return true;
            }
            else aterr(iAt, "lipseste ) dupa expresia \n");
        }
        else aterr(iAt, "lipseste expresia dupa ( \n");
    }

    iAt = start;
    return false;
}

int main() {
    FILE* fis;
    char ch;
    char* buff;
    int i = 0;

    fis = fopen("v1.c", "r");
    if (fis == NULL) {
        printf("eroare deschidere fisier\n");
        exit(1);
    }

    buff = (char*)malloc(sizeof(char) * 30000);
    if (buff == NULL) {
        printf("alocare incorecta\n");
        exit(1);
    }

    while ((ch = fgetc(fis)) != EOF) {
        buff[i++] = ch;
    }
    buff[i++] = '\0';

    fclose(fis);
    pch = buff;
    while (getNextAtom() != END) {}

    //showAtoms();

    pushDomain();
    iAt = atoms;
    unit();

    showDomain(symTable, "global");
    dropDomain();

    return 0;
}



