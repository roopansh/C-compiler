#include <bits/stdc++.h>

using namespace std;

extern int yylineno;

enum DataType{
	dt_none,
	dt_int,
	dt_float,
	dt_bool,
	dt_func
};

class Node {
private:
	string type;	// lexeme class
	string value;	// lexeme
	// string name;

	DataType data_type;

	int line_number;


public:
	// Children of the Nodes
	Node *child1;
	Node *child2;
	Node *child3;
	Node(string t, string v, Node *c1, Node *c2, Node *c3){
		type = t;
		value = v;
		data_type = dt_none;
		child3 = c3;
		child2 = c2;
		child1 = c1;
	}

	string getValue(){
		return value;
	}

	string getType(){
		return type;
	}


	DataType getDataType(){
		return data_type;
	}

	void setDataType(DataType dt){
		data_type = dt;
	}
	// ~Node();

};


class Parameter
{
private:
	string name;		// parameter name
	DataType data_type;	// parameter data type
public:
	Parameter(){}

	Parameter(string id, DataType dt)
	:name(id), data_type(dt)
	{}
	// ~Parameter();

};

class SymbolTableAux
{
private:
	DataType data_type;		// datatype of the symbol

	// if symbol is a function, then following are also required
	// i.e. data_type = dt_func
	DataType return_type;
	vector <Parameter> parameter_list;
	int parameter_count;

public:
	SymbolTableAux(){}

	SymbolTableAux(DataType dt):data_type(dt){}

	SymbolTableAux(DataType dt, DataType rtd, vector <Parameter> params)
	:data_type(dt), return_type(rtd), parameter_list(params), parameter_count(params.size())
	{}

	// ~SymbolTableAux();

};


class SymbolTable
{
private:
	int scope;	// current maximum scope
				// 0 => Global scope

	vector < map < string, SymbolTableAux > > symbols;	// vector of maps at different scopes. vector[i] => map of symbols at scope i
public:
	SymbolTable(){
		scope = 0;	// global
		symbols.push_back(map<string, SymbolTableAux>());	// empty symbols table at global scope
	}


	bool findInCurrentScope(string id){
		if(symbols[scope].find(id) !=  symbols[scope].end()){
			return true;
		} else {
			return false;
		}
	}

	bool find(string id){
		for (int i = 0; i <= scope; ++i)
		{
			if(symbols[i].find(id) !=  symbols[scope].end()){
				return true;
			}
		}
		return false;
	}

	void addVariableInCurrentScope(string id, DataType dt){
		symbols[scope][id] = SymbolTableAux(dt);
	}

	SymbolTableAux* addFunctionInCurrentScope(string id, DataType rdt, vector<Parameter> params){
		SymbolTableAux *temp;
		temp = new SymbolTableAux(dt_func, rdt, params);
		symbols[scope][id] = *temp;
		return temp;
	}

	void addScope(){
		scope++;
		symbols.push_back(map<string, SymbolTableAux>());
	}

	void removeScope(){
		scope--;
		symbols.pop_back();
	}


	// ~SymbolTable();

};



class SemanticAnalysis
{
private:
	int error_count;
	stringstream error_message;
	bool inside_loop;
	SymbolTable symtab;
	SymbolTableAux *active_fun_ptr;

public:

	SemanticAnalysis(Node *TreeRoot)
	:error_count(0), inside_loop(false), active_fun_ptr(NULL)
	{
		analyse(TreeRoot);
	}


	void analyse(Node *node){
		if(node == NULL)	return;

		string node_type = node->getType();

		if (node_type == "program"){
			// Analyse the children
			analyse(node->child1);	// declaration list
			analyse(node->child2);	// main function

		} else if (node_type == "declaration_list" ) {
			// Analyse the children
			analyse(node->child1);	// declaration list
			analyse(node->child2);	// declaration

		} else if (node_type == "declaration" ){
			// Analyse the children
			analyse(node->child1);	// variable/function declaration

		} else if (node_type == "variable_declaration" ){

			if(node->child3 == NULL){ 		// type variablelist
				// get variables from the child2
				vector<string> vars = expandVariablesList(node->child2);

				// check if variable not already declared
				// add if not declared
				for (std::vector<string>::iterator i = vars.begin(); i != vars.end(); ++i){
					if(symtab.findInCurrentScope(*i)){
						error_message<<"Variable already declared in line number "<< yylineno <<" : "<<*i <<endl;
						error_count++;
					} else {
						symtab.addVariableInCurrentScope(*i, node->child1->getDataType());
					}
				}
			} else { 		// type variable = expression
				analyse(node->child3);
				if(symtab.findInCurrentScope(node->child2->getValue())) {
					error_message<<"Variable already declared in line number "<< yylineno <<" : "<< node->child2->getValue()<<endl;
					error_count++;
				} else if (!checkDatatypeCoercible(node->child3->getDataType(), node->child1->getDataType())) {
					error_message<<"Type mismatch in line number "<< yylineno <<" : expected "<< node->child1->getDataType() << "passed " << node->child3->getDataType() <<endl;
					error_count++;
				} else {
					symtab.addVariableInCurrentScope(node->child2->getValue(), node->child1->getDataType());
				}
			}

		} else if (node_type == "variable" ){
			// Check if declared or not
			if(!symtab.findInCurrentScope(node->child1->getValue())) {
				error_count++;
				error_message << "Variable " << node->child1->getValue() <<" used before declaration in line number : "<< yylineno<<endl;
			}

		} else if (node_type == "function_declaration" ){
			if(!symtab.find(node->getValue())){ 	// declare the function if not already declared
				// add a scope
				symtab.addScope();
				vector <Parameter> params = expandParameterList(node->child2);
				// set the active function pointer
				active_fun_ptr = symtab.addFunctionInCurrentScope(node->getValue(), node->child1->getDataType(), params);
				// analyse the subtree
				analyse(node->child3);
				//remove the scope
				symtab.removeScope();
			} else {	// check for function overloading

						/*
						*
						*				TO BE IMPLEMENTED
						*
						*
						*
						*/
				error_count++;
				error_message << "Function Already declared."<<endl;
			}

		} else if (node_type == "main_function" ){
			// Analyse the children
			active_fun_ptr = symtab.addFunctionInCurrentScope("main", dt_none, vector<Parameter> ());
			analyse(node->child1);

		} else if (node_type == "" ){
			// Analyse the children

		} else if (node_type == "" ){
			// Analyse the children

		} else if (node_type == "" ){
			// Analyse the children

		} else if (node_type == "" ){
			// Analyse the children

		} else if (node_type == "" ){
			// Analyse the children

		} else if (node_type == "" ){
			// Analyse the children

		} else if (node_type == "" ){
			// Analyse the children

		} else if (node_type == "" ){
			// Analyse the children

		} else if (node_type == "" ){
			// Analyse the children

		} else if (node_type == "" ){
			// Analyse the children

		} else if (node_type == "" ){
			// Analyse the children
		} else if (node_type == "" ){
			// Analyse the children
		} else if (node_type == "" ){
			// Analyse the children
		} else if (node_type == "" ){
			// Analyse the children

			// Analyse the children
		} else { 	// ERROR
		}
	}


	vector<string> expandVariablesList(Node *tree){
		vector<string> res;
		if(tree->getType() != "variable_list"){
			return res;
		} else if(tree->child2 == NULL){
			res.push_back(tree->child1->getValue());
		} else {
			res.push_back(tree->child2->getValue());
			vector <string> temp = expandVariablesList(tree->child1);
			for (std::vector<string>::reverse_iterator i = temp.rbegin(); i != temp.rend(); ++i){
				res.insert(res.begin(), *i);
			}
		}
		return res;
	}

	vector <Parameter> expandParameterList(Node *tree){
		vector<Parameter> res;
		if(tree->getType() != "parameters" || tree->child1 == NULL){
			return res;
		} else {
			Node *paramlist = tree->child1;
			if(paramlist->child2 == NULL){
				res.push_back(Parameter(paramlist->child1->child2->getValue(), paramlist->child1->getDataType()));
			} else {
				res.push_back(Parameter(paramlist->child2->child2->getValue(), paramlist->child2->getDataType()));

				vector <Parameter> temp = expandParameterList(paramlist->child1);

				for (std::vector<Parameter>::reverse_iterator i = temp.rbegin(); i != temp.rend(); ++i){
					res.insert(res.begin(), *i);
				}
			}
		}
		return res;
	}

	bool checkDatatypeCoercible(DataType dt1, DataType dt2){
		if (dt1 == dt2) {
			return true;
		} else if((dt1 == dt_int || dt1 == dt_float) && ((dt2 == dt_int || dt2 == dt_float))){
			return true;
		} else {
			return false;
		}
	}

	// ~SemanticAnalysis();

	void errors(){
		if(error_count == 0){
			cout<<"No Semantic Errors"<<endl;
		} else {
			cout<<error_count<<" error(s) found!"<<endl;
			cout<<error_message.str()<<endl;
		}
	}

};
