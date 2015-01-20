//
//  Lexer.h
//  FunctionalPipeline
//
//  Created by Seán Hargadon on 14/01/2015.
//  Copyright (c) 2015 Seán Hargadon. All rights reserved.
//

#ifndef __FunctionalPipeline__Lexer__
#define __FunctionalPipeline__Lexer__

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <regex>

struct Token {
    std::string str;
    int value;
    int token;
    
    Token() {}
    Token(std::string str, int value, int token) {this->str = str; this->value = value; this->token = token;}
};

struct PreToken {
    int token_number;
    std::string regex_string;
    std::function<Token(PreToken, std::string)> f;
};

class Lexer {
private:
    std::string file;
    int token_number;
    
    bool newNumbers;
    
    std::string reg;
    
    std::vector<PreToken> pre_tokens;
    std::vector<Token> tokens;
    
    static std::vector<std::string> readFile(std::string file);
    void tokenise(std::vector<std::string> assembly);
    std::vector<std::string> getStringTokens(std::string str);
    
    char getCurrentChar(int current, std::string str);

public:
    Lexer();
    Lexer(std::string file);
    
    int create_token(std::string r, std::function<Token(PreToken, std::string)> f);
    int create_token(std::string r);
    void newNumbersToggle();


    std::vector<Token> getTokens();
};

#endif /* defined(__FunctionalPipeline__Lexer__) */
