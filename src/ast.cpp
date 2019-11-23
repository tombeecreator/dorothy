#include "../include/ast.hpp"
using namespace std;

void 
Node::addTab(ostream& os, int tab) {
    for(int i = 0; i < tab; i++) {
        os << "  ";
    }
}

void 
Expression::print(ostream& os, int tab) {
    os << "<EXP>";
}

void 
AssignSt::print(ostream& os, int tab) {
    Node::addTab(os, tab);
    os << _id << "=";
    _expr->print(os, tab);
    os << endl;
}

void 
AssignSt::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    ofs.push_back(Code::makeCode(Code::MOVE, 2, 0));
    ofs.push_back(Code::makeCode(Code::PUSHI, vars[_id], 0));
    ofs.push_back(Code::makeCode(Code::POP, 3, 0));
    ofs.push_back(Code::makeCode(Code::SUB, 0, 0));
    ofs.push_back(Code::makeCode(Code::PUSHR, 2, 0));
    _expr->compile(ofs, vars, functions);
    ofs.push_back(Code::makeCode(Code::POP, 3, 0));
    ofs.push_back(Code::makeCode(Code::POP, 2, 0));
    ofs.push_back(Code::makeCode(Code::STORE, 2, 3));
}

void
DeclVar::print(ostream& os, int tab) {
    os << "int " << _id;
}

void
DeclVar::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    if(!vars[_id]) { 
        vars[_id] = vars.size(); 
        ofs.push_back(Code::makeCode(Code::MOVE, 2, 0));
        ofs.push_back(Code::makeCode(Code::PUSHI, vars[_id], 0));
        ofs.push_back(Code::makeCode(Code::POP, 3, 0));
        ofs.push_back(Code::makeCode(Code::SUB, 0, 0));
        ofs.push_back(Code::makeCode(Code::MOVE, 1, 2));
        return;
    }
    throw CompileError((string("redeclared variable: ") + _id).c_str());
}

void 
DeclVarSt::print(ostream& os, int tab) {
    Node::addTab(os, tab);
    _decl->print(os, tab);
    os << endl;
}

void 
DeclVarSt::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    _decl->compile(ofs, vars, functions);
}

void 
IfSt::print(ostream& os, int tab) {
    Node::addTab(os, tab);
    os << "if(";
    _cond->print(os, tab);
    os << ") ";
    _truest->print(os, tab);
    if(_falsest) {
        Node::addTab(os, tab);
        os << "else ";
        _falsest->print(os, tab);
    }
    os << endl;
}

void 
IfSt::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    int jumpToElse;
    int jumpFromTrue;
    _cond->compile(ofs, vars, functions);
    ofs.push_back(Code::makeCode(Code::POP, 2, 0));
    ofs.push_back(Code::makeCode(Code::JNE, 0, 0));
    jumpToElse = ofs.size() - 1;
    _truest->compile(ofs, vars, functions);
    ofs[jumpToElse].op1 = ofs.size() + 4;
    if(_falsest) {
        ofs.push_back(Code::makeCode(Code::JMP, 0, 0));
        jumpFromTrue = ofs.size() - 1;
        _falsest->compile(ofs, vars, functions);
        ofs[jumpFromTrue].op1 = ofs.size() - 1 + 4;
    }
}

void 
WhileSt::print(ostream& os, int tab) {
    Node::addTab(os, tab);
    os << "while(";
    _cond->print(os, tab);
    os << ") ";
    _body->print(os, tab);
    os << endl;
}

void 
WhileSt::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    int jumpToBottom = 0;
    int top = ofs.size() - 1;
    _cond->compile(ofs, vars, functions);
    ofs.push_back(Code::makeCode(Code::POP, 2, 0));
    ofs.push_back(Code::makeCode(Code::JNE, 0, 0));
    jumpToBottom = ofs.size() - 1;
    _body->compile(ofs, vars, functions);
    ofs.push_back(Code::makeCode(Code::JMP, top, 0));
    ofs[jumpToBottom].op1 = ofs.size() - 1 + 4;
}

void 
Block::print(ostream& os, int tab) {
    os << "{" << endl;
    for(Statement *statement: _statements) {
        statement->print(os, tab+1);
    }
    Node::addTab(os, tab);
    os << "}" << endl;
}

void 
Block::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    for(Statement *statement: _statements) {
        statement->compile(ofs, vars, functions);
    }
}

void
CallFuncSt::print(ostream& os, int tab) {
    Node::addTab(os, tab);
    os << _id << "(";
    if(_args.size() > 0) {
        _args[0]->print(os, tab);
        for(int i = 1; i < _args.size(); i++) {
            _args[i]->print(os, tab);
        }
    }
    os << ")" << endl;
}

void
CallFuncSt::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    if(_args.size() > 0) {
        for(auto arg: _args) {
            arg->compile(ofs, vars, functions);
        }
    }
    if(functions[_id]) {
        ofs.push_back(Code::makeCode(Code::CALL, functions[_id], 0));
        ofs.push_back(Code::makeCode(Code::PUSHR, 2, 0));
    } else {
        throw CompileError(format("undefined function: %s", _id.c_str()).c_str());
    }
}

void
Function::print(ostream& os, int tab) {
    Node::addTab(os, tab);
    os << "func " << _id << "(";
    if(_args.size() > 0) {
        _args[0]->print(os, tab);
        for(int i = 1; i < _args.size(); i++) {
            os << ", ";
            _args[i]->print(os, tab);
        }
    }
    os << ")";
    _body->print(os, tab);
}

void
Function::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    vars.clear();
    functions[_id] = ofs.size() + 3;
    ofs.push_back(Code::makeCode(Code::PUSHR, 0, 0));
    ofs.push_back(Code::makeCode(Code::MOVE, 0, 1));
    if(_args.size() > 0) {
        for(auto arg: _args) {
            arg->compile(ofs, vars, functions);
        }
        for(int i = 0; i < _args.size(); i++) {
            ofs.push_back(Code::makeCode(Code::MOVE, 2, 0));
            ofs.push_back(Code::makeCode(Code::MOVEI, 3, i + 2));
            ofs.push_back(Code::makeCode(Code::ADD, 0, 0));
            ofs.push_back(Code::makeCode(Code::LOAD, 2, 2));
            ofs.push_back(Code::makeCode(Code::PUSHR, 2, 0));
            ofs.push_back(Code::makeCode(Code::MOVE, 2, 0));
            ofs.push_back(Code::makeCode(Code::MOVEI, 3, _args.size() - i));
            ofs.push_back(Code::makeCode(Code::SUB, 0, 0));
            ofs.push_back(Code::makeCode(Code::POP, 3, 0));
            ofs.push_back(Code::makeCode(Code::STORE, 2, 3));
        }
    }
    _body->compile(ofs, vars, functions);
    ofs.push_back(Code::makeCode(Code::PUSHI, 0, 0));
    ofs.push_back(Code::makeCode(Code::POP, 2, 0));
    ofs.push_back(Code::makeCode(Code::MOVE, 1, 0));
    ofs.push_back(Code::makeCode(Code::POP, 3, 0));
    ofs.push_back(Code::makeCode(Code::MOVE, 0, 3));
    ofs.push_back(Code::makeCode(Code::RET, 0, 0));
}

void
ReturnSt::print(ostream& os, int tab) {
    Node::addTab(os, tab);
    os << "return ";
    _exp->print(os, tab);
    os << endl;
}

void
ReturnSt::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    _exp->compile(ofs, vars, functions);
    ofs.push_back(Code::makeCode(Code::POP, 2, 0));
    ofs.push_back(Code::makeCode(Code::MOVE, 1, 0));
    ofs.push_back(Code::makeCode(Code::POP, 3, 0));
    ofs.push_back(Code::makeCode(Code::MOVE, 0, 3));
    ofs.push_back(Code::makeCode(Code::RET, 0, 0));
}

void
AddExp::print(ostream& os, int tab) {
    _left->print(os, tab);
    os << "+";
    _right->print(os, tab);
}

void
AddExp::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    _left->compile(ofs, vars, functions);
    _right->compile(ofs, vars, functions);
    ofs.push_back(Code::makeCode("POP 3 0"));
    ofs.push_back(Code::makeCode("POP 2 0"));
    ofs.push_back(Code::makeCode("ADD 0 0"));
    ofs.push_back(Code::makeCode("PUSHR 2 0"));
}

void
SubExp::print(ostream& os, int tab) {
    _left->print(os, tab);
    os << "-";
    _right->print(os, tab);
}

void
SubExp::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    _left->compile(ofs, vars, functions);
    _right->compile(ofs, vars, functions);
    ofs.push_back(Code::makeCode("POP 3 0"));
    ofs.push_back(Code::makeCode("POP 2 0"));
    ofs.push_back(Code::makeCode("SUB 0 0"));
    ofs.push_back(Code::makeCode("PUSHR 2 0"));
}

void
MulExp::print(ostream& os, int tab) {
    _left->print(os, tab);
    os << "*";
    _right->print(os, tab);
}

void
MulExp::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    _left->compile(ofs, vars, functions);
    _right->compile(ofs, vars, functions);
    ofs.push_back(Code::makeCode(Code::POP, 2, 0));
    ofs.push_back(Code::makeCode(Code::POP, 3, 0));
    ofs.push_back(Code::makeCode(Code::MUL, 0, 0));
    ofs.push_back(Code::makeCode(Code::PUSHR, 2, 0));
}

void
DivExp::print(ostream& os, int tab) {
    _left->print(os, tab);
    os << "/";
    _right->print(os, tab);
}

void
DivExp::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    _left->compile(ofs, vars, functions);
    _right->compile(ofs, vars, functions);
    ofs.push_back(Code::makeCode("POP 3 0"));
    ofs.push_back(Code::makeCode("POP 2 0"));
    ofs.push_back(Code::makeCode("DIV 0 0"));
    ofs.push_back(Code::makeCode("PUSHR 2 0"));
}

void
ModExp::print(ostream& os, int tab) {
    _left->print(os, tab);
    os << "%";
    _right->print(os, tab);
}

void
ModExp::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    _left->compile(ofs, vars, functions);
    _right->compile(ofs, vars, functions);
    ofs.push_back(Code::makeCode("POP 3 0"));
    ofs.push_back(Code::makeCode("POP 2 0"));
    ofs.push_back(Code::makeCode("MOD 0 0"));
    ofs.push_back(Code::makeCode("PUSHR 2 0"));
}

void
EQExp::print(ostream& os, int tab) {
    _left->print(os, tab);
    os << "==";
    _right->print(os, tab);
}

void
EQExp::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    _left->compile(ofs, vars, functions);
    _right->compile(ofs, vars, functions);
    ofs.push_back(Code::makeCode("POP 3 0"));
    ofs.push_back(Code::makeCode("POP 2 0"));
    ofs.push_back(Code::makeCode("EQ 0 0"));
    ofs.push_back(Code::makeCode("PUSHR 2 0"));
}

void
NEExp::print(ostream& os, int tab) {
    _left->print(os, tab);
    os << "!=";
    _right->print(os, tab);
}

void
NEExp::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    _left->compile(ofs, vars, functions);
    _right->compile(ofs, vars, functions);
    ofs.push_back(Code::makeCode("POP 3 0"));
    ofs.push_back(Code::makeCode("POP 2 0"));
    ofs.push_back(Code::makeCode("NE 0 0"));
    ofs.push_back(Code::makeCode("PUSHR 2 0"));
}

void
LTExp::print(ostream& os, int tab) {
    _left->print(os, tab);
    os << "<";
    _right->print(os, tab);
}

void
LTExp::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    _left->compile(ofs, vars, functions);
    _right->compile(ofs, vars, functions);
    ofs.push_back(Code::makeCode("POP 3 0"));
    ofs.push_back(Code::makeCode("POP 2 0"));
    ofs.push_back(Code::makeCode("LT 0 0"));
    ofs.push_back(Code::makeCode("PUSHR 2 0"));
}

void
LEExp::print(ostream& os, int tab) {
    _left->print(os, tab);
    os << "<=";
    _right->print(os, tab);
}

void
LEExp::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    _left->compile(ofs, vars, functions);
    _right->compile(ofs, vars, functions);
    ofs.push_back(Code::makeCode("POP 3 0"));
    ofs.push_back(Code::makeCode("POP 2 0"));
    ofs.push_back(Code::makeCode("LE 0 0"));
    ofs.push_back(Code::makeCode("PUSHR 2 0"));
}

void
GTExp::print(ostream& os, int tab) {
    _left->print(os, tab);
    os << ">";
    _right->print(os, tab);
}

void
GTExp::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    _left->compile(ofs, vars, functions);
    _right->compile(ofs, vars, functions);
    ofs.push_back(Code::makeCode("POP 3 0"));
    ofs.push_back(Code::makeCode("POP 2 0"));
    ofs.push_back(Code::makeCode("GT 0 0"));
    ofs.push_back(Code::makeCode("PUSHR 2 0"));
}

void
GEExp::print(ostream& os, int tab) {
    _left->print(os, tab);
    os << ">=";
    _right->print(os, tab);
}

void
GEExp::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    _left->compile(ofs, vars, functions);
    _right->compile(ofs, vars, functions);
    ofs.push_back(Code::makeCode("POP 2 0"));
    ofs.push_back(Code::makeCode("POP 3 0"));
    ofs.push_back(Code::makeCode("GE 0 0"));
    ofs.push_back(Code::makeCode("PUSHR 2 0"));
}

void
IntExp::print(ostream& os, int tab) {
    os << _int_val;
}

void
IntExp::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    ofs.push_back(Code::makeCode("PUSHI " + to_string(_int_val) + " 0"));
}

void
VarExp::print(ostream& os, int tab) {
    os << _id;
}

void
VarExp::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    ofs.push_back(Code::makeCode("MOVE 2 0"));
    ofs.push_back(Code::makeCode("PUSHI " + to_string(vars[_id]) + " 0"));
    ofs.push_back(Code::makeCode("POP 3 0"));
    ofs.push_back(Code::makeCode("SUB 0 0"));
    ofs.push_back(Code::makeCode("LOAD 2 2"));
    ofs.push_back(Code::makeCode("PUSHR 2 0"));
}

void
CallFuncExp::print(ostream& os, int tab) {
    os << _id << "(";
    if(_args.size() > 0) {
        _args[0]->print(os, tab);
        for(int i = 1; i < _args.size(); i++) {
            _args[i]->print(os, tab);
        }
    }
    os << ")";
}

void
CallFuncExp::compile(vector<Code>& ofs, map<string, int>& vars, map<string, int>& functions) {
    if(_args.size() > 0) {
        for(auto arg: _args) {
            arg->compile(ofs, vars, functions);
        }
    }
    if(functions[_id]) {
        ofs.push_back(Code::makeCode(Code::CALL, functions[_id], 0));
        ofs.push_back(Code::makeCode(Code::PUSHR, 2, 0));
    } else {
        throw CompileError(format("undefined function: %s", _id.c_str()).c_str());
    }
}
