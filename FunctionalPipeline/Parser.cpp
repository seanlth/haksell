//
//  Parser.cpp
//  FunctionalPipeline
//
//  Created by Seán Hargadon on 14/01/2015.
//  Copyright (c) 2015 Seán Hargadon. All rights reserved.
//

#include "Parser.h"

#define error(s) std::cout << s << std::endl; for (Function* f : this->functions) delete f; exit(1);

FunctionCall::~FunctionCall()
{
    for (Expr* e : this->expr_list) delete e;
}

EList::~EList()
{
    delete this->expr;
}


Parser::Parser()
{
    
}

Parser::Parser(std::string input, std::string output)
{
    this->symbol_table.push_back( Symbol("+", "number", {"number", "number"}, nullptr) );
    this->symbol_table.push_back( Symbol("-", "number", {"number", "number"}, nullptr) );
    this->symbol_table.push_back( Symbol("*", "number", {"number", "number"}, nullptr) );
    this->symbol_table.push_back( Symbol("/", "number", {"number", "number"}, nullptr) );

    
    Lexer l = Lexer(input);
    
    this->token_newline = l.create_token ("\\n"); //newlines
    this->token_whitespace = l.create_token ("\\s+"); // whitespace
    this->token_comments = l.create_token ("\\/\\/.*"); //commments
    this->token_assign = l.create_token ("="); //assign
    this->token_lparen = l.create_token ("\\("); //assign
    this->token_rparen = l.create_token ("\\)"); //assign
    this->token_lbrace = l.create_token ("\\{"); //assign
    this->token_rbrace = l.create_token ("\\}"); //assign


    //this->token_equal = l.create_token ("=="); //equal
    //this->token_le = l.create_token ("<="); //less than or equal
    //this->token_ge = l.create_token (">="); //greater than or equal
    //this->token_lt = l.create_token ("<"); //less than
    //this->token_gt = l.create_token (">");
    
    //this->token_add = l.create_token ("\\+");
    //this->token_sub = l.create_token ("\\-");
    //this->token_mul = l.create_token ("\\*");
    //this->token_div = l.create_token ("\\/");
    this->token_string = l.create_token ("\"[^\"^\n^r]+\"",  [] (PreToken t, std::string x) -> Token { return Token(x.substr(1, x.length()-2), 0, t.token_number); } ); //string
    this->token_comma = l.create_token (",");
    this->token_at = l.create_token ("@");
    l.newNumbersToggle();
    this->token_number = l.create_token ("0[b][01]+", [] (PreToken t, std::string x) -> Token { return Token(x, (int)strtoull(x.c_str(), NULL, 2), t.token_number); });
    this->token_number = l.create_token ("0[x][0-9a-fA-F]+", [] (PreToken t, std::string x) -> Token { return Token(x, (int)strtoull(x.c_str(), NULL, 16), t.token_number); });
    l.newNumbersToggle();
    this->token_number = l.create_token ("[0-9]+", [] (PreToken t, std::string x) -> Token { return Token(x, atoi(x.c_str()), t.token_number); } );

    this->token_ident = l.create_token("[a-zA-Z]+"); //ident
    this->token_underscore = l.create_token("_"); //ident
    this->token_binop = l.create_token ( "[^\\w^\\n^\\s]+"); //binop
    this->token_error = l.create_token (".+"); //error
    
    this->token_eof = -1;
    
    this->tokens = l.getTokens();
    
    parseProgram();
    
    std::cout << this->functions[0]->code_gen("");
    
    std::ofstream f;
    
    this->file = output;
    
    f.open(this->file);
    
    if (!f.is_open()) {
        std::cout << "Error opening file" << std::endl;
        exit(-1);
    }
    
    
    f.close();
    
}

Token Parser::getCurrentToken(int currentToken)
{
    Token tok;
    if (currentToken >= 0 && currentToken < this->tokens.size()) {
        tok = this->tokens[currentToken];
    }
    else if ( currentToken >= this->tokens.size() ) {
        return Token("EOF", 0, this->token_eof);
    }
    
    return tok;
}

void Parser::addToSymbolTable(Symbol s)
{
    bool found = false;
    for (Symbol sm : this->symbol_table) {
        if (sm.name.compare(s.name) == 0) {
            found = true;
        }
    }
    if (!found) {
        this->symbol_table.push_back( s );
    }
}

Symbol Parser::lookUpSymbolTable(std::string name)
{
    Symbol s;
    bool found = false;
    for (Symbol sm : this->symbol_table) {
        if (sm.name.compare(name) == 0) {
            s = sm;
            found = true;
            break;
        }
    }
    if (!found) {
        error("Reference to undeclared variable");
    }
    
    return s;
}

void Parser::updateSymbolTable(Symbol s)
{
    bool found = false;
    for (int i = 0; i < this->symbol_table.size(); i++ ) {
        if (this->symbol_table[i].name.compare(s.name) == 0) {
            this->symbol_table[i] = s;
            found = true;
            break;
        }
    }
    if (!found) {
        error("Reference to undeclared variable");
    }
}

FunctionCall* Parser::searchSymbolTable(std::string name)
{
    FunctionCall* f = nullptr;
    
    for (Symbol sm : this->symbol_table) {
        if (sm.name.compare(name) == 0) {
            f = sm.f;
            break;
        }
    }
    
    return f;
}


void Parser::parseProgram()
{
    parseFunctionList();
}

void Parser::parseFunctionList()
{
    if ( getCurrentToken(this->currentToken).token == token_ident ) {
        this->functions.push_back( parseFunction() );
        if (getCurrentToken(this->currentToken).token == token_newline) {
            this->currentToken++;
            parseNewlineList();
            parseFunctionList();
        }
    }
    else if (getCurrentToken(this->currentToken).token == token_comments || getCurrentToken(this->currentToken).token == token_newline) {
        this->currentToken++;
        parseFunctionList();
    }
    else if ( getCurrentToken(this->currentToken).token != token_eof ) {
        error("Expected eof, found " + getCurrentToken(this->currentToken).str);
    }
}

void Parser::parseNewlineList()
{
    while (getCurrentToken(this->currentToken).token == token_newline) {
        this->currentToken++;
    }
}

Function* Parser::parseFunction()
{
    Function* f = new Function();
    
    f->name = parseIdent();
    f->arg_list = parseArgList();
    
    if ( getCurrentToken(this->currentToken).token == token_assign ) {
        this->currentToken++;
        f->expr = parseExpression();
    }
    else {
        error("Expected '=', found " + getCurrentToken(this->currentToken).str);
    }
    
    std::vector<std::string> arg_types;
    
    for (Primary* p : f->arg_list) {
        if (p->primary_type == Primary::primary_functionCall) {
            arg_types.push_back( lookUpSymbolTable( (((FunctionCall*)p)->ident )).return_type );
        }
    }
    
    updateSymbolTable( Symbol(f->name->ident, f->expr->type, arg_types, f->name) );
    
    f->type = f->expr->type;
    
    return f;
}

Expr* Parser::parseExpression()
{
    Expr* e = new Expr();
    
    e->part = parsePart();
    e->e_list = parseEList();
    
    if (e->e_list == nullptr) {
        e->type = e->part->type;
    }
    else {
        e->type = lookUpSymbolTable(e->e_list->binop).return_type;
    }
    
    return e;
}

EList* Parser::parseEList()
{
    EList* el = nullptr;
    
    if (getCurrentToken(this->currentToken).token == token_binop) {
        el = new EList();
        el->binop = parseBinop();
        el->expr = parseExpression();
        
        if (el->expr->part->e_list == nullptr) {
            el->expr->part->primary->type = lookUpSymbolTable(el->binop).args_types[1];
            updateSymbolTable( Symbol(((FunctionCall*)el->expr->part->primary)->ident, el->expr->part->primary->type, {}, (FunctionCall*)el->expr->part->primary) );
        }
    }
    
    return el;
}

Part* Parser::parsePart()
{
    Part* p = nullptr;
    
    if (getCurrentToken(this->currentToken).token == token_lparen) {
        this->currentToken++;
        p = parsePart();
        if (getCurrentToken(this->currentToken).token != token_rparen) {
            error("Expected ')', found " + getCurrentToken(this->currentToken).str);
        }
        this->currentToken++;
    }
    else if (getCurrentToken(this->currentToken).token == token_string ||
             getCurrentToken(this->currentToken).token == token_number ||
             getCurrentToken(this->currentToken).token == token_lbrace ||
             getCurrentToken(this->currentToken).token == token_ident) {
        p = new Part();
        p->primary = parsePrimary();
        p->e_list = parseEList();
        
        //set type of left op
        if (p->e_list != nullptr) {
            p->type = lookUpSymbolTable(p->e_list->binop).return_type;
            if (p->primary->primary_type == Primary::primary_functionCall) {
                p->primary->type = lookUpSymbolTable(p->e_list->binop).args_types[0];
                updateSymbolTable( Symbol(((FunctionCall*)p->primary)->ident, p->primary->type, {}, ((FunctionCall*)p->primary) ) );
            }
            else {
                if ( p->primary->type.compare( lookUpSymbolTable(p->e_list->binop).args_types[0] ) ) {
                    error("Type mismatch in " + p->e_list->binop );
                }
            }
        }
        else {
            p->type = p->primary->type;
        }
    }
    else {
        error("Expected '(' or primary, found " + getCurrentToken(this->currentToken).str);
    }
    
    return p;
}


std::vector<Expr*> Parser::parseExprList()
{
    std::vector<Expr*> expr_list;
    
    expr_list.push_back( parseExpression() );
    std::vector<Expr*> more = parseExprListMore();
    
    for (Expr* e : more) { expr_list.push_back(e); }
    
    return expr_list;
}

std::vector<Expr*> Parser::parseExprListMore()
{
    std::vector<Expr*> expr_list_more;
    
    while (getCurrentToken(this->currentToken).token == token_string ||
           getCurrentToken(this->currentToken).token == token_number ||
           getCurrentToken(this->currentToken).token == token_lbrace ||
           getCurrentToken(this->currentToken).token == token_ident ||
           getCurrentToken(this->currentToken).token == token_lparen) {
        expr_list_more.push_back( parseExpression() );
    }
    
    return expr_list_more;
}

Primary* Parser::parsePrimary()
{
    Primary* p = nullptr;
    
    if (getCurrentToken(this->currentToken).token == token_string) {
        p = parseString();
    }
    else if (getCurrentToken(this->currentToken).token == token_number) {
        p = parseNumber();
    }
    else if (getCurrentToken(this->currentToken).token == token_lbrace) {
        this->currentToken++;
        parseArray();
    }
    else if (getCurrentToken(this->currentToken).token == token_ident) {
        p = parseFunctionCall();
    }
    else {
        error("Expected primary, found " + getCurrentToken(this->currentToken).str);
    }
    
    return p;
}

FunctionCall* Parser::parseFunctionCall()
{
    FunctionCall* fc;
    
    fc = parseIdent();
    fc->expr_list = parseExprListMore();
    fc->type = lookUpSymbolTable(fc->ident).return_type;
    
    return fc;
}

String* Parser::parseString()
{
    String* s = new String();
    
    if (getCurrentToken(this->currentToken).token == token_string) {
        s->str = getCurrentToken(this->currentToken).str;
        this->currentToken++;
    }
    
    return s;
}

Number* Parser::parseNumber()
{
    Number* n = new Number();
    
    if (getCurrentToken(this->currentToken).token == token_number) {
        n->value = getCurrentToken(this->currentToken).value;
        this->currentToken++;
    }
    
    return n;
}

FunctionCall* Parser::parseIdent()
{
    FunctionCall* ident = nullptr;
    
    if (getCurrentToken(this->currentToken).token == token_ident) {
        ident = searchSymbolTable(getCurrentToken(this->currentToken).str);
        
        if (ident == nullptr) {
            ident = new FunctionCall();
            ident->ident = getCurrentToken(this->currentToken).str;
            addToSymbolTable( Symbol(ident->ident, "", {}, ident) );
        }
        
        this->currentToken++;
    }
    else {
        error("Expected ident, found " + getCurrentToken(this->currentToken).str);
    }
    
    
    return ident;
}

Array* Parser::parseArray()
{
    Array* a = new Array();
    
    return a;
}

Primary* Parser::parseArg()
{
    Primary* p = nullptr;
    
    if (getCurrentToken(this->currentToken).token == token_string) {
        p = parseString();
    }
    else if (getCurrentToken(this->currentToken).token == token_number) {
        p = parseNumber();
    }
    else if (getCurrentToken(this->currentToken).token == token_lbrace) {
        this->currentToken++;
        parseArrayArg();
    }
    else if (getCurrentToken(this->currentToken).token == token_ident) {
        p = parseIdent();
    }
    else if (getCurrentToken(this->currentToken).token == token_underscore) {
        this->currentToken++;
    }
    else {
        error("Expected arg, found " + getCurrentToken(this->currentToken).str);
    }
    
    return p;

}

std::vector<Primary*> Parser::parseArgList()
{
    std::vector<Primary*> al;
    
    if (getCurrentToken(this->currentToken).token == token_string ||
        getCurrentToken(this->currentToken).token == token_number ||
        getCurrentToken(this->currentToken).token == token_ident ||
        getCurrentToken(this->currentToken).token == token_lbrace ||
        getCurrentToken(this->currentToken).token == token_underscore) {
        
        al.push_back( parseArg() );
        
        while (getCurrentToken(this->currentToken).token == token_string ||
               getCurrentToken(this->currentToken).token == token_number ||
               getCurrentToken(this->currentToken).token == token_ident ||
               getCurrentToken(this->currentToken).token == token_lbrace ||
               getCurrentToken(this->currentToken).token == token_underscore) {
            al.push_back( parseArg() );
        }
    }
    
    return al;
}

void Parser::parseArrayArg()
{
    
}

std::string Parser::parseBinop()
{
    std::string b;
    
    if (getCurrentToken(this->currentToken).token == token_binop) {
        b = getCurrentToken(this->currentToken).str;
        this->currentToken++;
    }
    else {
        error("Expected binop, found " + getCurrentToken(this->currentToken).str);
    }
    
    return b;
}


