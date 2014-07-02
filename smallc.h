union content{
    float Float;
    int Int;
    char Char;    
    
};

typedef struct dataStruct{
    int type;
    char name[256];
    union content value; 
    int ifUsed;
    struct dataStruct* next;
} dataStruct, *dataNode;
dataNode head;
dataNode tail;


typedef struct valueStruct{
    int type; 
    char contents[256];
} valueStruct, *valueNode;

typedef struct midCodeStruct{
    int line;
    int opt;
    valueNode value1;
    valueNode value2;
    int jumpLine;
    struct midCodeStruct* next;
    struct midCodeStruct* ifNext;
    struct midCodeStruct* whileNext;

}midCodeStruct, *midCodeNode;

midCodeNode midHead;
midCodeNode midTail;

enum Opt{
    Add,
    Sub,
    Div,
    Multi,
    Mod,
    Mov,
    Jump,
    CMP,
    JA,
    JB,
    JAE,
    JBE,
    JE,
    JNE,
};


enum ErrorType{
    ValueExisted
};

enum ValueType{
    IdentType,
    NumberType,
    Char,
    Int,
    Float,
    JumpType,
};
extern char *strdup(const char *s);
int line;
extern valueNode exeLexp(mpc_ast_t* t);
extern valueNode exeTerm(mpc_ast_t* t);
extern int exe(mpc_ast_t* t);
extern int pushIfNode(midCodeNode node);
midCodeNode ifStackHead;
midCodeNode ifStackTail;

midCodeNode whileStackHead;
midCodeNode whileStackTail;

typedef struct whileLoopNode{
    int line;
    struct whileLoopNode* next;
}whileLoopNodeStruct, * whileLoopNode;

whileLoopNode  whileLoopHead;
whileLoopNode  whileLoopTail;

