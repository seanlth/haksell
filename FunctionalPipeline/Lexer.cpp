    //
//  Lexer.cpp
//  FunctionalPipeline
//
//  Created by Seán Hargadon on 14/01/2015.
//  Copyright (c) 2015 Seán Hargadon. All rights reserved.
//

#include "Lexer.h"


Lexer::Lexer()
{
    
}

Lexer::Lexer(std::string file)
{
    this->file = file;
    this->newNumbers = false;
}


std::vector<std::string> Lexer::readFile(std::string file)
{
    std::vector<std::string> result;
    
    std::ifstream f(file);
    std::string output;
    if (f.is_open()) {
        while (std::getline(f, output))
        {
            result.push_back(output);
        }
    }
    else {
        std::cout << "Error opening file" << std::endl;
        exit(-1);
    }
    f.close();
    
    return result;
}

std::vector<Token> Lexer::getTokens()
{
    this->reg = this->pre_tokens[0].regex_string;
    
    for (int i = 1; i < this->pre_tokens.size(); i++) {
        this->reg += "|" + this->pre_tokens[i].regex_string;
    }

    tokenise( this->readFile(file) );

    return this->tokens;
}

std::vector<std::string> Lexer::getStringTokens(std::string str)
{
    std::vector<std::string> toks;
    
    std::regex r(this->reg);
    
    std::smatch m;
    
    regex_search(str, m, r);
    
    while (std::regex_search (str, m, r)) {
        for (auto x:m) if (x.str()[0] != ' ') toks.push_back(x);
        str = m.suffix().str();
    }
    toks.push_back("\n");
    
    return toks;
}

void Lexer::tokenise(std::vector<std::string> assembly)
{
    for (std::string s : assembly) {
        std::string instruction = s;
        auto string_tokens = getStringTokens(instruction);
        for (auto x : string_tokens) {
            for (PreToken t : this->pre_tokens) {
                std::regex_match(x, std::regex(t.regex_string));
                
                if ( std::regex_match(x, std::regex(t.regex_string)) ) {
                    tokens.push_back( t.f(t, x) );
                    break;
                }
            }
        }
    }
    this->tokens.push_back(Token("EOF", 0, -1 ));
}


int Lexer::create_token(std::string r, std::function<Token(PreToken, std::string)> f)
{
    PreToken t;
    t.regex_string = r;
    t.token_number = this->token_number;
    t.f = f;
    
    this->pre_tokens.push_back(t);
    
    return this->newNumbers ? this->token_number : this->token_number++;
}

int Lexer::create_token(std::string r)
{
    PreToken t;
    t.regex_string = r;
    t.token_number = this->token_number;
    t.f = [] (PreToken t, std::string x) -> Token { return Token( x, 0, t.token_number ); };
    
    this->pre_tokens.push_back(t);
    
    return this->newNumbers ? this->token_number : this->token_number++;
}

void Lexer::newNumbersToggle()
{
    if (this->newNumbers) {
        this->newNumbers = false;
    }
    else {
        this->newNumbers = true;
    }
}





