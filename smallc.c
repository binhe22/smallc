#include "mpc.h"
#include "string.h"
#include "stdio.h"
#include "smallc.h"
////////////////////data node////////////////////////////////////
dataNode createListNode(){
    dataNode node = (dataNode)malloc(sizeof(dataStruct));
    memset(node, 0, sizeof(dataStruct));
    node->next = NULL;
    return node;
}

dataNode searchListNode(char *name){
    dataNode tmp = head;
    while(tmp->next){
        if(!strcmp(name, tmp->next->name)){
            return tmp->next;
        }
        tmp = tmp->next;        
    }

    return NULL;
}

int insertListNode(dataNode node){
    if(searchListNode(node->name)){
        printf("value %s existed", node->name);
        return -1;
    }
    tail->next = node;
    tail = tail->next;
    return 0;
}
///////////////////////mid code list/////////////////////////////////
midCodeNode createMidNode(){
    midCodeNode node = (midCodeNode)malloc(sizeof(midCodeStruct));
    if(!node) return NULL;
    memset(node, 0, sizeof(midCodeStruct));
    return node;
}

int insertMidNode(midCodeNode node){
    line = line+1;
    midTail->next = node;
    node->next = NULL;
    node->line = line;
    midTail = midTail->next;
    return 0;
}
/////////////////////////////////////////////////////////////////////////////

mpc_ast_t* searchAstByTag(mpc_ast_t* t, char *tag){
    int i;
    for(i=0; i<t->children_num; i++){
        if(!strcmp(t->children[i]->tag, tag)){
            return t->children[i];  
        }
    }
    return NULL;
    
}


int translateType(char *type){
    if(!strcmp(type,"int"))
        return Int;
    else if(!strcmp(type, "char"))
        return Char;
    else if(!strcmp(type, "float"))
        return Float;

    return -1;
}

int translateOptType(char *opt){
    if(!strcmp(opt, "==")) return JNE;
    if(!strcmp(opt, ">=")) return JB;
    if(!strcmp(opt, "<=")) return JA;
    if(!strcmp(opt, "<"))  return JAE;
    if(!strcmp(opt, ">"))  return JBE;
    if(!strcmp(opt, "!="))  return JE;
    switch(opt[0]){
        case '+':
        return Add;
        case '-':
        return Sub;
        case '*':
        return Multi;
        case '/':
        return Div;
        case '%':
        return Mod;
    }
    return -1;
}

int getTypeByName(char *name){
    dataNode node = searchListNode(name);
    if(!node) return -1;
    return node->type;
}

midCodeNode handleOpt(valueNode opt1, valueNode opt2, int optType){
    midCodeNode node = (midCodeNode)malloc(sizeof(midCodeStruct));
    if(!node) return NULL;
    node->opt = optType;
    node->value1 = opt1;
    node->value2 = opt2;
    insertMidNode(node);
    return node;
}


int insertDeclsListNode(mpc_ast_t* t){
    dataNode node= createListNode();
    node->type = translateType(t->children[0]->contents);
    strcpy(node->name, t->children[1]->contents);
    node->ifUsed = 0;
    insertListNode(node);
    return 0;
}

int createDeclsList(mpc_ast_t* t){
    int i;
    for(i=0; i<t->children_num; i++){
        if(!strcmp(t->children[i]->tag, "typeident|>")){
            insertDeclsListNode(t->children[i]);        
        }
    }   
    return 0;
}


valueNode createValueNode(int type, char *content){
    valueNode node = malloc(sizeof(valueStruct));
    node->type = type;
    strcpy(node->contents, content);
    return node;
}


valueNode exeFactor(mpc_ast_t* t){
    valueNode v;
    v = (valueNode)malloc(sizeof(valueStruct));
    if(strstr(t->tag, "number")){
        strcpy(v->contents, t->contents);    
        v->type = NumberType;
        return v;
    }

    else if(strstr(t->tag, "ident")){
        strcpy(v->contents, t->contents);
        v->type = IdentType;
        return v;
    }
    else{
        if(strstr(t->children[1]->tag, "lexp"))
            return exeLexp(t->children[1]);
        else if(strstr(t->children[1]->tag, "term"))
            return exeTerm(t->children[1]);
        else if(strstr(t->children[1]->tag, "factor"))
            return exeFactor(t->children[1]);
    }
    return NULL;
}



valueNode exeTerm(mpc_ast_t* t){
    int i;
    valueNode node = NULL;
    valueNode tmp = NULL;
    int opt = -1;

    for(i=0; i<t->children_num; i++){
        if(strstr(t->children[i]->tag,"factor")){
            tmp = exeFactor(t->children[i]);
            if(i == 0){
                node = tmp;
            }
            else{
                handleOpt(node, tmp, opt);
            }
        }
        else{
            opt = translateOptType(t->children[i]->contents);
        }
    }

    if(!t->children_num){
        tmp = exeFactor(t);
        return tmp;


    }
    return node;
}

valueNode exeLexp(mpc_ast_t* t){
    int i;
    valueNode node = NULL;
    valueNode tmp = NULL;
    int opt = -1;

    for(i=0; i<t->children_num; i++){
        if(strstr(t->children[i]->tag,"term")){
            tmp = exeTerm(t->children[i]);
            if(i == 0){
                node = tmp;
            }
            else{
                handleOpt(node, tmp, opt);
            }
        }
        else if(strstr(t->children[i]->tag,"factor")){
            tmp = exeFactor(t->children[i]);
            if(i == 0){
                node = tmp;
            }
            else{
                handleOpt(node, tmp, opt);
            }
        }
        else{
            opt = translateOptType(t->children[i]->contents);
        }
    }
    if(!t->children_num){
        tmp = exeTerm(t);
        return tmp;
    }
    
    return node;

}



valueNode exeNormal(mpc_ast_t* t){

    char lValue[256] = {0};
    strcpy(lValue, t->children[0]->contents);
        
    valueNode tmp = NULL;
    if(strstr(t->children[2]->tag, "lexp")) tmp = exeLexp(t->children[2]);
    else if(strstr(t->children[2]->tag, "term")) tmp = exeTerm(t->children[2]);
    else if(strstr(t->children[2]->tag, "factor")) tmp = exeFactor(t->children[2]);
    else return NULL;
    valueNode node = createValueNode(IdentType, lValue);
    handleOpt(node, tmp, Mov);
    return node;

}



int popLoopNode(){
    if(whileLoopHead == whileLoopTail) return -1;
    whileLoopNode tmp = whileLoopHead;

    while(tmp->next->next){
        tmp = tmp->next;
    }
    whileLoopNode returnNode = tmp->next;
    tmp->next = NULL;
    whileLoopTail = tmp;
    return returnNode->line;
}

int pushLoopNode(int line){
    whileLoopNode node = (whileLoopNode)malloc(sizeof(whileLoopNodeStruct));
    node->line = line;

    whileLoopTail->next = node;
    node->next = NULL;
    whileLoopTail = whileLoopTail->next;
    return 0;
}





midCodeNode popIfNode(){
    if(ifStackHead == ifStackTail) return NULL;
    midCodeNode tmp = ifStackHead;
    while(tmp->ifNext->ifNext){
        tmp = tmp->ifNext;
    }
    midCodeNode returnNode = tmp->ifNext;
    tmp->ifNext = NULL;
    ifStackTail = tmp;
    return returnNode;
}

int pushIfNode(midCodeNode node){
    ifStackTail->ifNext = node;
    node->ifNext = NULL;
    ifStackTail = ifStackTail->ifNext;
    return 0;
}



midCodeNode popWhileNode(){
    if(whileStackHead == whileStackTail) return NULL;
    midCodeNode tmp = whileStackHead;
    while(tmp->whileNext->whileNext){
        tmp = tmp->whileNext;
    }
    midCodeNode returnNode = tmp->whileNext;
    tmp->whileNext = NULL;
    whileStackTail = tmp;
    return returnNode;
}

int pushWhileNode(midCodeNode node){
    whileStackTail->whileNext = node;
    node->whileNext = NULL;
    whileStackTail = whileStackTail->whileNext;
    return 0;
}




int exeExp(mpc_ast_t* t, int type){
    
    handleOpt(exeLexp(t->children[0]), exeLexp(t->children[2]), CMP);
    
    valueNode value1, value2;
    value1 = (valueNode)malloc(sizeof(valueStruct));    
    value2 = (valueNode)malloc(sizeof(valueStruct));    
    value1->type = JumpType;
    strcpy(value1->contents, "noyet");
    if(type == 0) strcpy(value2->contents, "if ");
    if(type == 1) strcpy(value2->contents, "while ");

    int opt = translateOptType(t->children[1]->contents);
    
    if (type == 0) pushIfNode(handleOpt(value1, value2, opt));
    if (type == 1) pushWhileNode(handleOpt(value1, value2, opt));
    return 0;
}


int exeIf(mpc_ast_t* t){
    exeExp(t->children[2], 0);
    exe(t->children[4]);
    midCodeNode node = popIfNode();
    if(!node){
        printf("if handle stack error\n");
    }
    sprintf(node->value1->contents, "%d", line+1);
    return 0;
}


int exeWhileLoop(){
    valueNode value1, value2;
    value1 = (valueNode)malloc(sizeof(valueStruct));    
    value2 = (valueNode)malloc(sizeof(valueStruct));    
    value1->type = JumpType;
    strcpy(value1->contents, "noyet");
    strcpy(value2->contents, "while loop");
    int line = popLoopNode();
    sprintf(value1->contents, "%d", line);
    handleOpt(value1, value2, Jump);
    return 0;
 
}

int exeWhile(mpc_ast_t* t){
    pushLoopNode(line+1);
    exeExp(t->children[2], 1);
    exe(t->children[4]);
    midCodeNode node = popWhileNode();
    if(!node){
        printf("while handle statck error\n");
    }
    sprintf(node->value1->contents, "%d", line+2);
    exeWhileLoop(); 
    return 0;
}

int exe(mpc_ast_t* t){
    int i;
    mpc_ast_t* node;

    for(i=0; i<t->children_num; i++){
        node = t->children[i];
        if(strstr(node->tag, "stmt")){
            if(!strcmp(node->children[1]->contents, "=")){
                exeNormal(node);
            }
        

            else if(!strcmp(node->children[0]->contents, "if")){
                exeIf(node);
            }

            else if(!strcmp(node->children[0]->contents, "while")){
                exeWhile(node);
            }
        }
    }
    if(strstr(t->children[0]->tag, "ident")){
        if(!strcmp(t->children[1]->contents, "=")){
                exeNormal(t);
            }
        

            else if(!strcmp(t->children[0]->contents, "if")){
                exeIf(t);
            }

            else if(!strcmp(t->children[0]->contents, "while")){
                exeWhile(t);
            }
    }


    return 0;
}

int handleBody(mpc_ast_t* t){
    mpc_ast_t* declsNode = searchAstByTag(t, "decls|>");
    if(!declsNode){
        printf("Error no define node\n");
        return -1;
    }
    if(createDeclsList(declsNode) == -1){
        printf("create DeclsList error\n");
        return -1;
    }
    exe(t);
    return 0;

}



int handleMain(mpc_ast_t* t){
    mpc_ast_t* astNode = searchAstByTag(t, "body|>");
    if(astNode){
        handleBody(astNode);
        return 0;
    }
    printf("Main function has no body;\n");
    return -1;
}


int eval(mpc_ast_t* t) {
  
  /* If tagged as number return it directly, otherwise expression. */ 
    mpc_ast_t* astNode = searchAstByTag(t, "main|>");
    if(astNode){
        handleMain(astNode);
        return 0;
    }
    printf("No main function\n");
    return -1;
}


int printMidCode(){
    midCodeNode tmp =  midHead;
    while(tmp->next){
        printf("line %d: %d %s %s\n", tmp->next->line, tmp->next->opt, tmp->next->value1->contents, tmp->next->value2->contents);
        tmp = tmp->next;
    }
    return 0;
}

int printDataList(){
    dataNode tmp =  head;
    while(tmp->next){
        printf("%d %s\n", tmp->next->type, tmp->next->name);
        tmp = tmp->next;
    }
    return 0;
}

int main(int argc, char **argv) {
    line = 0;
    head = createListNode();
    tail = head;
    midHead = createMidNode();
    midTail = midHead;
    
    ifStackHead = createMidNode();
    ifStackTail = ifStackHead;

    whileStackHead = createMidNode();
    whileStackTail = whileStackHead;

    whileLoopHead = (whileLoopNode)malloc(sizeof(whileLoopNodeStruct));
    whileLoopTail =  whileLoopHead;


      mpc_parser_t* Ident     = mpc_new("ident");
      mpc_parser_t* Number    = mpc_new("number");
      mpc_parser_t* Character = mpc_new("character");
      mpc_parser_t* String    = mpc_new("string");
      mpc_parser_t* Factor    = mpc_new("factor");
      mpc_parser_t* Term      = mpc_new("term");
      mpc_parser_t* Lexp      = mpc_new("lexp");
      mpc_parser_t* Stmt      = mpc_new("stmt");
      mpc_parser_t* Exp       = mpc_new("exp");
      mpc_parser_t* Typeident = mpc_new("typeident");
      mpc_parser_t* Decls     = mpc_new("decls");
      mpc_parser_t* Args      = mpc_new("args");
      mpc_parser_t* Body      = mpc_new("body");
      mpc_parser_t* Procedure = mpc_new("procedure");
      mpc_parser_t* Main      = mpc_new("main");
      mpc_parser_t* Includes  = mpc_new("includes");
      mpc_parser_t* Smallc    = mpc_new("smallc");

      mpc_err_t* err = mpca_lang(MPCA_LANG_DEFAULT,
        " ident     : /[a-zA-Z_][a-zA-Z0-9_]*/ ;                           \n"
        " number    : /[-]?[0-9]+(\\.[0-9]+)?/;                                           \n"
        " character : /'.'/ ;                                              \n"
        " string    : /\"(\\\\.|[^\"])*\"/ ;                               \n"
        "                                                                  \n"
        " factor    : '(' <lexp> ')'                                       \n"
        "           | <number>                                             \n"
        "           | <character>                                          \n"
        "           | <string>                                             \n"
        "           | <ident> '(' <lexp>? (',' <lexp>)* ')'                \n"
        "           | <ident> ;                                            \n"
        "                                                                  \n"
        " term      : <factor> (('*' | '/' | '%') <factor>)* ;             \n"
        " lexp      : <term> (('+' | '-') <term>)* ;                       \n"
        "                                                                  \n"
        " stmt      : '{' <stmt>* '}'                                      \n"
        "           | \"while\" '(' <exp> ')' <stmt>                       \n"
        "           | \"if\"    '(' <exp> ')' <stmt>                       \n"
        "           | <ident> '=' <lexp> ';'                               \n"
        "           | <ident> \"++\" ';'                               \n"
        "           | <ident> \"--\" ';'                               \n"
        "           | \"print\" '(' <lexp>? ')' ';'                        \n"
        "           | \"return\" <lexp>? ';'                               \n"
        "           | <ident> '(' <ident>? (',' <ident>)* ')' ';' ;        \n"
        "                                                                  \n"
        " exp       : <lexp> '>' <lexp>                                    \n"
        "           | <lexp> '<' <lexp>                                    \n"
        "           | <lexp> \">=\" <lexp>                                 \n"
        "           | <lexp> \"<=\" <lexp>                                 \n"
        "           | <lexp> \"!=\" <lexp>                                 \n"
        "           | <lexp> \"==\" <lexp> ;                               \n"
        "                                                                  \n"
        " typeident : (\"int\" | \"char\"|\"float\") <ident> ;                       \n"
        " decls     : (<typeident> ';')* ;                                 \n"
        " args      : <typeident>? (',' <typeident>)* ;                    \n"
        " body      : '{' <decls> <stmt>* '}' ;                            \n"
        " procedure : (\"int\" | \"char\") <ident> '(' <args> ')' <body> ; \n"
        " main      : \"main\" '(' ')' <body> ;                            \n"
        " includes  : (\"#include\" <string>)* ;                           \n"
        " smallc    : /^/ <includes> <decls> <procedure>* <main> /$/ ;     \n",
        Ident, Number, Character, String, Factor, Term, Lexp, Stmt, Exp, 
        Typeident, Decls, Args, Body, Procedure, Main, Includes, Smallc, NULL);
      
      if (err != NULL) {
        mpc_err_print(err);
        mpc_err_delete(err);
        exit(1);
      }
        
      if (argc > 1) {
        
        mpc_result_t r;
        if (mpc_parse_contents(argv[1], Smallc, &r)) {
          mpc_ast_print(r.output);
          eval(r.output);
          mpc_ast_delete(r.output);
        } else {
          mpc_err_print(r.error);
          mpc_err_delete(r.error);
        }
        
      } else {
        
        mpc_result_t r;
        if (mpc_parse_pipe("<stdin>", stdin, Smallc, &r)) {
          mpc_ast_print(r.output);
          mpc_ast_delete(r.output);
        } else {
          mpc_err_print(r.error);
          mpc_err_delete(r.error);
        }
      
      }
    printDataList();
     printMidCode();

    mpc_cleanup(17, Ident, Number, Character, String, Factor, Term, Lexp, Stmt, Exp,
                      Typeident, Decls, Args, Body, Procedure, Main, Includes, Smallc);
      
      return 0;
      
}




