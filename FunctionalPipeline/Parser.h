//
//  Parser.h
//  FunctionalPipeline
//
//  Created by Seán Hargadon on 14/01/2015.
//  Copyright (c) 2015 Seán Hargadon. All rights reserved.
//

#ifndef __FunctionalPipeline__Parser__
#define __FunctionalPipeline__Parser__

#include <stdio.h>
#include "Lexer.h"


class Expr;

struct Primary {
    
    enum Primary_type {
        primary_string, primary_number, primary_functionCall, primary_array
    };
    
    Primary_type primary_type;
    std::string type;
    
    Primary() {}
    
    virtual std::string code_gen()
    {
        return "";
    }
};

struct Array : public Primary {
    std::vector<Primary*> primary_list;
    Array() { this->primary_type = primary_array; }
    ~Array() {  for (Primary* p : this->primary_list) delete p; }
    
    std::string code_gen()
    {
        return "";
    }
};


struct String : public Primary {
    std::string str;
    String() { this->primary_type = primary_string; this->type = "string"; }
    
    
    std::string code_gen()
    {
        return "\"" + str + "\"";
    }
};


struct Number : public Primary {
    int value;
    Number() { this->primary_type = primary_number; this->type = "number"; }
    
    std::string code_gen()
    {
        return std::to_string(value);
    }
};

struct FunctionCall : public Primary {
    std::string ident;
    std::vector<Expr*> expr_list;
    
    FunctionCall() { this->primary_type = primary_functionCall; }
    ~FunctionCall();
    
    std::string code_gen()
    {
        return "";
    }
};


//<>


struct EList {
    std::string binop;
    Expr* expr;
    EList() {}
    ~EList();
    
    std::string code_gen()
    {
        std::string el;
        
        el += binop;
        
        return el;
    }
};

struct Part {
    std::string type;
    Primary* primary;
    EList* e_list;
    Part() {}
    ~Part() { delete e_list; }
    
    std::string code_gen()
    {
        std::string p;
        
        p += this->code_gen();
        
        if (e_list != nullptr) {
            p += e_list->code_gen();
        }
        
        return p;
    }
};

struct Expr {
    std::string type;
    Part* part;
    EList* e_list;
    Expr() {};
    ~Expr() { delete part; delete e_list; }
    
    std::string code_gen()
    {
        std::string e;
        
        e += this->part->code_gen();
        
        if (e_list != nullptr) {
            e += this->e_list->code_gen();
        }
        
        return e;
    }
};


struct Function {
    std::string type;
    FunctionCall* name;
    std::vector<Primary*> arg_list;
    Expr* expr;
    
    Function() {}
    ~Function() { delete name; delete expr; for (Primary *p : arg_list) delete p; }
    
    std::string code_gen(std::string output)
    {
        std::string f;
        
        f = this->type + " " + this->name->ident + " ( ";
        
        f += (this->arg_list[0]->type) + " " + static_cast<FunctionCall*>(this->arg_list[0])->ident;
        
        for (int i = 1; i < this->arg_list.size(); i++) {
            if (this->arg_list[i]->primary_type == Primary::primary_functionCall) {
                f += ", " + (this->arg_list[i]->type) + " " + static_cast<FunctionCall*>(this->arg_list[i])->ident;
            }
        }
        
        f += " ) { ";
        
        
        return f;
    }
};

struct Symbol {
    std::string name;
    std::string return_type;
    std::vector<std::string> args_types;
    FunctionCall* f;
    
    Symbol() {}
    Symbol(std::string name, std::string type, std::vector<std::string> args_types, FunctionCall* f) { this->name = name; this->return_type = type; this->args_types = args_types; this->f = f; }
};

class Parser {
private:
    std::vector<Function*> functions;
    
    std::vector<Token> tokens;
    int currentToken;
    
    std::string file;
    
    std::vector<Symbol> symbol_table;
    
    int token_ident;
    int token_number;
    int token_string;
    
    int token_assign;
    int token_lparen;
    int token_rparen;
    int token_lbrace;
    int token_rbrace;
    
    int token_add;
    int token_sub;
    int token_mul;
    int token_div;
    
    int token_equal;
    int token_le;
    int token_ge;
    int token_lt;
    int token_gt;
    
    int token_comma;
    int token_at;
    int token_binop;
    int token_underscore;
    
    int token_newline;
    int token_whitespace;
    int token_comments;
    int token_error;
    int token_eof;
    
    
    
    void parseProgram();
    void parseFunctionList();
    Function* parseFunction();
    void parseNewlineList();
    Expr* parseExpression();
    EList* parseEList();
    Part* parsePart();
    std::vector<Expr*> parseExprList();
    std::vector<Expr*> parseExprListMore();
    Primary* parsePrimary();
    FunctionCall* parseFunctionCall();
    String* parseString();
    Number* parseNumber();
    FunctionCall* parseIdent();
    Array* parseArray();
    Primary* parseArg();
    std::vector<Primary*> parseArgList();
    void parseArrayArg();
    std::string parseBinop();

    Token getCurrentToken(int currentToken);
    
    void addToSymbolTable(Symbol s);
    Symbol lookUpSymbolTable(std::string name);
    FunctionCall* searchSymbolTable(std::string name);
    void updateSymbolTable(Symbol s);
    
    
public:
    Parser();
    Parser(std::string input, std::string output);
    
};


#endif /* defined(__FunctionalPipeline__Parser__) */
