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
            if(isalpha(ch)||ch=='_'){ pCrtCh++; state=33; // consuma caracterul si trece la noua stare
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
bool exprPrimary(){
    Token *start = iTk;
    if(consume(ID)){
        if (consume(LPAR)){
            if(expr()){
                while(consume(COMMA)){
                    if(expr()){}
                    else tkerr(iTk,"lipseste o expresie dupa virgula");
                }
            }
            if(consume(RPAR)){
                return true;
            }
        }
        return true;

    }
    if(consume(CT_INT)){
        return true;
    }
    if(consume(CT_REAL)){
        return true;
    }
    if(consume(CT_CHAR)){
        return true;
    }
    if(consume(CT_STRING)){
        return true;
    }
    if(consume(LPAR)){
        if (expr()){
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
bool exprPostfixPrim(){
    Token *start = iTk;
    if(consume(LBRACKET)){
        if(expr()){
            if(consume(RBRACKET)){
                if(exprPostfixPrim()){
                    return true;
                }
            }
        }
    }
    if(consume(DOT)){
        if(consume(ID)){
            if(exprPostfixPrim()){
                return true;
            }
        }
    }
    return true;
}
bool exprPostfix(){
    Token *start = iTk;
    if(exprPrimary()){
        if(exprPostfixPrim()){
            return true;
        }
    }
    iTk = start;
    return false;
}

///exprUnary: (SUB | NOT) exprUnary | exprPostfix
bool exprUnary(){
    Token *start = iTk;
    if(consume(SUB)){
        if(exprUnary()){
            return true;
        } else tkerr(iTk,"lipseste o expresie unara de dupa semnul -");
    }
    if (consume(NOT)){
        if(exprUnary()){
            return true;
        } else tkerr(iTk,"lipseste o expresie unara de dupa semnul !");
    }
    if(exprPostfix()){
        return true;
    }
    iTk = start;
    return false;
}

bool typeBase();
bool arrayDecl();

///exprCast: LPAR typeBase arrayDecl? RPAR exprCast | exprUnary
bool exprCast(){
    Token *start = iTk;
    if(consume(LPAR)){
        if(typeBase()){
            if(arrayDecl()){}
            if (consume(RPAR)){
                if(exprCast()){
                    return true;
                } else tkerr(iTk,"lipseste o expresie dupa )");
            }else tkerr(iTk,"lipseste ) dupa specificarea tipului");
        } else tkerr(iTk,"lipseste specificarea tipului dupa ( ");
    }
    if(exprUnary()){
        return true;
    }
    iTk = start;
    return false;
}

///exprMul: exprMul (MUL | DIV) exprCast | exprCast
///exprMul: exprCast exprMulPrim
///exprMulPrim: (MUL | DIV) exprCast exprMulPrim | epsilon
bool exprMulPrim(){
    Token *start = iTk;
    if(consume(MUL) || consume(DIV)){
        if(exprCast()){
            if (exprMulPrim()){
                return true;
            }
        }
    }
    return true;
}
bool exprMul(){
    Token *start = iTk;
    if(exprCast()){
        if(exprMulPrim()){
            return true;
        }
    }
    iTk = start;
    return false;
}

///exprAdd: exprAdd (ADD | SUB) exprMul | exprMul
///exprAdd: exprMul exprAddPrim
///exprAddPrim: (ADD | SUB) exprMul exprAddPrim | epsilon
bool exprAddPrim(){
    Token *start = iTk;
    if(consume(ADD) || consume(SUB)){
        if(exprMul()){
            if (exprAddPrim()){
                return true;
            }
        }
    }
    return true;
}
bool exprAdd(){
    Token *start = iTk;
    if(exprMul()){
        if(exprAddPrim()){
            return true;
        }
    }
    iTk = start;
    return false;
}

///exprRel: exprRel (LESS | LESSEQ | GREATER | GREATEREQ) exprAdd | exprAdd
///exprRel: exprAdd exprRelPrim
///exprRelPrim: (LESS | LESSEQ | GREATER | GREATEREQ) exprAdd exprRelPrim | epsilon
bool exprRelPrim(){
    Token *start = iTk;
    if(consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)){
        if(exprAdd()){
            if (exprRelPrim()){
                return true;
            }
        }
    }
    return true;
}
bool exprRel(){
    Token *start = iTk;
    if(exprAdd()){
        if(exprRelPrim()){
            return true;
        }
    }
    iTk = start;
    return false;
}

///exprEq: exprEq (EQUAL | NOTEQ) exprRel | exprRel
///exprEq: exprRel exprEqPrim
///exprEqPrim: (EQUAL | NOTEQ) exprRel exprEqPrim | epsilon
bool exprEqPrim(){
    Token *start = iTk;
    if(consume(EQUAL) || consume(NOTEQ)){
        if(exprRel()){
            if (exprEqPrim()){
                return true;
            }
        }
    }
    return true;
}
bool exprEq(){
    Token *start = iTk;
    if(exprRel()){
        if(exprEqPrim()){
            return true;
        }
    }
    iTk = start;
    return false;
}

///exprAnd: exprAnd AND exprEq | exprEq
///exprAnd: exprEq exprAndPrim
///exprAndPrim: AND exprEq exprAndPrim | epsilon
bool exprAndPrim(){
    Token *start = iTk;
    if(consume(AND)){
        if(exprEq()){
            if (exprAndPrim()){
                return true;
            }
        }
    }
    return true;
}
bool exprAnd(){
    Token *start = iTk;
    if(exprEq()){
        if(exprAndPrim()){
            return true;
        }
    }
    iTk = start;
    return false;
}

///exprOr: exprOr OR exprAnd | exprAnd
///exprOr: exprAnd exprOrPrim
///exprOrPrim: OR exprAnd exprOrPrim | epsilon
bool exprOrPrim(){
    Token *start = iTk;
    if(consume(OR)){
        if(exprAnd()){
            if (exprOrPrim()){
                return true;
            }
        }
    }
    return true;
}
bool exprOr(){
    Token *start = iTk;
    if(exprAnd()){
        if(exprOrPrim()){
            return true;
        }
    }
    iTk = start;
    return false;
}

///exprAssign: exprUnary ASSIGN exprAssign | exprOr
bool exprAssign(){
    Token *start = iTk;
    if(exprUnary()){
        if(consume(ASSIGN)){
            if(exprAssign()){
                return true;
            } tkerr(iTk,"lipseste dupa semnul = o expresia de atribuire");
        } else tkerr(iTk,"lipseste semnul = ");
    }
    if(exprOr()){
        return true;
    }
    iTk = start;
    return false;
}

///expr: exprAssign
bool expr(){
    Token *start = iTk;
    if(exprAssign()){
        return true;
    }
    iTk = start;
    return false;
}

bool varDef();
bool stm();

///stmCompound: LACC ( varDef | stm )* RACC
bool stmCompound(){
    Token *start = iTk;
    if(consume(LACC)){
        while(varDef() || stm()){ }
        if(consume(RACC)){
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
    if(stmCompound()){
        return true;
    }
    if(consume(IF)){
        if(consume(LPAR)){
            if(expr()){
                if(consume(RPAR)){
                    if(stm()){
                        if(consume(ELSE)){
                            if(stm()){
                                return true;
                            }else tkerr(iTk,"lipseste corpul de dupa else");
                        }
                        return true;
                    } else tkerr(iTk,"lipseste corpul de dupa )");
                } else tkerr(iTk,"lipseste )");
            } else tkerr(iTk,"lipseste o expresie dupa (");
        } else tkerr(iTk,"lipseste (");
    }
    if(consume(WHILE)){
        if(consume(LPAR)){
            if(expr()){
                if(consume(RPAR)){
                    if(stm()){
                        return true;
                    } else tkerr(iTk,"lipseste corpul din interiorul lui while");
                } else tkerr(iTk,"lipseste )");
            } else tkerr(iTk,"lipseste expresia de dupa (");
        } else tkerr(iTk,"lipseste (");
    }
    if(consume(FOR)){
        if(consume(LPAR)){
            if(expr()){}
            if(consume(SEMICOLON)){
                if(expr()){}
                if (consume(SEMICOLON)){
                    if(expr()){}
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
    if(consume(RETURN)){
        if(expr()){}
        if(consume(SEMICOLON)){
            return true;
        } else tkerr(iTk,"lipseste ; dupa return");
    }
    if(expr()){ }
    if(consume(SEMICOLON)){
        return true;
    } else tkerr(iTk,"lipseste ;");
    iTk = start;
    return false;
}

///fnParam: typeBase ID arrayDecl?
bool fnParam(){
    Token *start = iTk;
    if(typeBase()){
        if(consume(ID)){
            if(arrayDecl()) return true;
            return true;
        } else tkerr(iTk,"lipseste numele de dupa specificarea tipului");
    }
    iTk = start;
    return false;
}

///fnDef: ( typeBase | VOID ) ID LPAR ( fnParam ( COMMA fnParam )* )? RPAR stmCompound
bool fnDef(){
    Token *start = iTk;
    if(typeBase() || consume(VOID)){
        if(consume(ID)){
            if(consume(LPAR)){
                if(fnParam()){
                    while(consume(COMMA)){
                        if(fnParam()){}
                        else tkerr(iTk,"lipseste parametrul de dupa virgula");;
                    }
                    return true;
                }
                if(consume(RPAR)){
                    if(stmCompound()){
                        return true;
                    } else tkerr(iTk,"lipseste corpul");
                } else tkerr(iTk,"lipseste )");
            } else tkerr(iTk,"lipseste (");
        } else tkerr(iTk,"lipseste numele de dupa specificarea tipului");
    }
    iTk = start;
    return false;
}

///arrayDecl: LBRACKET CT_INT? RBRACKET
bool arrayDecl(){
    Token *start = iTk;
    if(consume(LBRACKET)){
        if(consume(CT_INT)){}
        if(consume(RBRACKET)){
            return true;
        } else tkerr(iTk,"lipseste ]");
    }
    iTk = start;
    return false;
}

///typeBase: INT | DOUBLE | CHAR | STRUCT ID
bool typeBase(){
    Token *start = iTk;
    if(consume(INT) || consume(DOUBLE) || consume(CHAR)){
        return true;
    }
    if(consume(STRUCT)){
        if(consume(ID)){
            return true;
        } else tkerr(iTk,"lipseste un nume pentru declararea structurii");
    }
    iTk = start;
    return false;
}

///varDef: typeBase ID arrayDecl? SEMICOLON
bool varDef(){
    Token *start = iTk;
    if(typeBase()){
        if(consume(ID)){
            if(arrayDecl()){}
            if (consume(SEMICOLON)){
                return true;
            } else tkerr(iTk,"lipseste ;");
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
            if (consume(LACC)){
                while(varDef()){}
                if(consume(RACC)){
                    if(consume(SEMICOLON)){
                        return true;
                    } else tkerr(iTk,"lipseste ;");
                }else tkerr(iTk,"lipseste }");
            }else tkerr(iTk,"lipseste {");
        } else tkerr(iTk,"lipseste numele tipului struct");
    }
    iTk = start;
    return false;
}

///unit: ( structDef | fnDef | varDef )* END
bool unit(){
    Token *start = iTk;
    while(structDef() || fnDef() || varDef() ){}
    if(consume(END)){
        return true;
    }
    iTk = start;
    return false;
}

void ASIN(){

}

int main(){
    FILE *fp;
    fp = fopen("fis1.txt", "r");
    int count;
    char ch;
	while((ch = fgetc(fp)) != EOF) {
		count++;
		printf("%c",ch);
	}
	fclose(fp);
	char *strng;
	if((strng=(char*)malloc((count + 1)*sizeof(char)))==NULL)err("not enough memory");
	fp = fopen("fis1.txt", "r");
	int i = 0;
	while((ch = fgetc(fp)) != EOF) {
		strng[i++] = ch;
	}
	strng[i] = '\0';
    pCrtCh = strng;
    char *ends = strng + i;
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

	ASIN();

	fclose(fp);
	return 0;
}

// dupa punct la ct real err
// daca pch[0] = si && pch[1] = si => e ok, altfel err
// daca pch[0] = or && pch[1] = or => e ok, altfel err


/*

// program de testare a analizorului lexical, v1.1

int main()
{
	int i;
	for(i=0;i<10;i=i+1){
		if(i/2==1)puti(i);
		}
	if(4.9==49e-1&&0.49E1==2.45*2.0)puts("yes");
	putc('#');
	puts("");	// pentru \n
	return 0;
}

*/


