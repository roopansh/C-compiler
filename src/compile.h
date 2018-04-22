#include <bits/stdc++.h>
#define DEBUG if(0)
using namespace std;

extern int yylineno;


enum DataType{
	dt_none,
	dt_int,
	dt_float,
	dt_bool,
	dt_func,
	dt_err
};


// Nodes of the AST
class Node {
private:
	string type;			// lexeme class
	string value;			// lexeme
	DataType data_type;		// datatype of the node(if required)

public:
	int line_number;		// line number where the node is occuring

	// Children of the Nodes
	Node *child1;
	Node *child2;
	Node *child3;

	Node (string t, string v, Node *c1, Node *c2, Node *c3) {
		type = t;
		value = v;
		data_type = dt_none;
		child3 = c3;
		child2 = c2;
		child1 = c1;
		line_number = yylineno;
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


// Parameter of a function
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

	DataType getDataType(){
		return data_type;
	}

	string getValue(){
		return name;
	}

	// ~Parameter();
};


// Class for the Meta data of the symbol table
class SymbolTableAux
{
private:
	DataType data_type;		// datatype of the symbol

	// if symbol is a function, then following are also required - return data type, parameter list, number of parameters
	// i.e. data_type = dt_func
	DataType return_type;
	vector <Parameter> parameter_list;
	int parameter_count;

public:

	SymbolTableAux(){

	}

	SymbolTableAux(DataType dt)
	:data_type(dt) {

	}

	SymbolTableAux(DataType dt, DataType rtd, vector <Parameter> params)
	:data_type(dt), return_type(rtd), parameter_list(params), parameter_count(params.size()) {

	}

	DataType getDataType(){
		return data_type;
	}

	DataType getReturnDataType(){
		return return_type;
	}

	vector<Parameter> getParameterList(){
		return parameter_list;
	}

	int getParameterCount(){
		return parameter_count;
	}

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
		for (int i = scope; i >= 0; i--)
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

	SymbolTableAux* addFunction(string id, DataType rdt, vector<Parameter> params){
		symbols[0][id] = SymbolTableAux(dt_func, rdt, params);
		return &symbols[0][id];
	}

	void addScope(){
		scope++;
		map<string, SymbolTableAux> newMap;
		newMap.clear();
		symbols.push_back(newMap);
	}

	void removeScope(){
		if(scope == 0)	return ;
		scope--;
		symbols.pop_back();
	}

	DataType getDataType(string id){
		for (int i = scope; i >= 0; i--)
		{
			if(symbols[i].find(id) !=  symbols[i].end()){
				return (symbols[i].find(id))->second.getDataType();
			}
		}
		return dt_none;
	}

	DataType getFunctionDataType(string id){
		for (int i = scope; i >= 0; i--)
		{
			if(symbols[i].find(id) !=  symbols[scope].end()){
				return (symbols[i].find(id))->second.getReturnDataType();
			}
		}
		return dt_none;
	}

	bool checkFunctionArgs(string id, vector<DataType> args_list) {
		for (int i = scope; i >= 0; i--)
		{
			if(symbols[i].find(id) !=  symbols[scope].end()){
				SymbolTableAux temp = symbols[i].find(id)->second;
				if(temp.getDataType() != dt_func){continue;}
				if(temp.getParameterCount() != args_list.size()){continue;}
				bool flag = true;
				int x = 0;
				for(vector <Parameter>::iterator i = temp.getParameterList().begin(); i != temp.getParameterList().end() && x < args_list.size(); i++, x++){
					if (i->getDataType() != args_list[x]){
						flag = false;
						break;
					}
				}
				if(flag){
					return true;
				}
			}
		}
		return false;

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
		DEBUG cerr << node_type<<endl;

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
						error_message<<"Line Number "<< node->line_number << " : Variable '"<< *i <<"' already declared." <<endl;
						error_count++;
						node->setDataType(dt_err);
					} else {
						symtab.addVariableInCurrentScope(*i, node->child1->getDataType());
					}
				}
			} else { 		// type variable = expression
				analyse(node->child3);
				if(symtab.findInCurrentScope(node->child2->getValue())) {
					error_message<<"Line Number "<< node->line_number << " : Variable '"<< node->child2->getValue() <<"' already declared." <<endl;
					error_count++;
					node->setDataType(dt_err);
				} else if (!checkDatatypeCoercible(node->child3->getDataType(), node->child1->getDataType())) {
					error_message<<"Line Number "<< node->line_number << " : Type mismatch. Expected "<< node->child1->getDataType() << " but passed " << node->child3->getDataType()<<endl;
					error_count++;
					node->setDataType(dt_err);
				} else {
					symtab.addVariableInCurrentScope(node->child2->getValue(), node->child1->getDataType());
				}
			}

		} else if (node_type == "variable" ){
			// Check if declared or not
			if(!symtab.find(node->child1->getValue())) {
				error_count++;
				error_message<<"Line Number "<< node->line_number<< " : Variable " << node->child1->getValue() <<" used before declaration."<<endl;
				node->setDataType(dt_err);
			}else {
				node->setDataType(symtab.getDataType(node->child1->getValue()));

			}

		} else if (node_type == "function_declaration" ){
			if(!symtab.find(node->getValue())){ 	// declare the function if not already declared
				symtab.addScope();
				// analyse the subtree
				vector <Parameter> params = expandParameterList(node->child2);
				for (std::vector<Parameter>::iterator i = params.begin(); i != params.end(); ++i)
				{
					symtab.addVariableInCurrentScope(i->getValue(), i->getDataType());
				}
				// set the active function pointer
				active_fun_ptr = symtab.addFunction(node->getValue(), node->child1->getDataType(), params);
				// add a scope
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
				error_message << "Line Number "<< node->line_number<<" : Function Already declared."<<endl;
				node->setDataType(dt_err);
			}

		} else if (node_type == "main_function" ){
			// Analyse the children
			active_fun_ptr = symtab.addFunction("main", dt_none, vector<Parameter> ());
			analyse(node->child1);

		} else if (node_type == "statements" ){
			// Analyse the children statements
			analyse(node->child1);
			analyse(node->child2);

		} else if (node_type == "statement" ){
			// Analyse the children
			analyse(node->child1);

		} else if (node_type == "condition" ){
			// Analyse the children
			analyse(node->child1);

			symtab.addScope();
			analyse(node->child2);
			symtab.removeScope();

			if(node->child3 != NULL){
				symtab.addScope();
				analyse(node->child3);
				symtab.removeScope();
			}

		} else if (node_type == "loop" ){
			// Analyse the children
			analyse(node->child1);

		} else if (node_type == "for_loop" ){
			// Analyse the children
			analyse(node->child1);
			analyse(node->child2);
			if(node->child1->getDataType() == node->child2->getDataType() == dt_int){
				symtab.addScope();
				analyse(node->child3);
				symtab.removeScope();
			} else {
				error_count++;
				error_message<<"Line Number "<<node->line_number<<" : Needs integer arguments in for loop"<<endl;
				node->setDataType(dt_err);
			}

		} else if (node_type == "while_loop" ){
			// Analyse the children
			analyse(node->child1);
			symtab.addScope();
			analyse(node->child2);
			symtab.removeScope();

		} else if (node_type == "return_statement" ){
			// Analyse the children
			if (active_fun_ptr == NULL) {
				error_count++;
				error_message<<"Line Number "<<node->line_number<<" : Return statement can only be used inside a function."<<endl;
				node->setDataType(dt_err);
			} else {
				if (node->child2->getDataType() == active_fun_ptr->getReturnDataType()) {
					active_fun_ptr = NULL;
				} else if (node->child2 == NULL && active_fun_ptr->getReturnDataType() == dt_none){
					active_fun_ptr = NULL;
				} else {
					error_count++;
					error_message<<"Line Number "<<node->line_number<<" : Function returns wrong data type."<<endl;
				}
			}

		} else if (node_type == "read" || node_type == "write" ){
			// Analyse the children
			analyse(node->child1);

		} else if (node_type == "expression" ){
			// Analyse the children
			if(node->child2 != NULL){
				analyse(node->child2);
				analyse(node->child1);

				if(!checkDatatypeCoercible(node->child1->getDataType(), node->child2->getDataType())){
					error_count++;
					error_message<<"Line Number "<<node->line_number<<" : Type mismatch. Unable to type cast implicitly.(expression)"<<endl;
					node->setDataType(dt_err);
				} else {
					node->setDataType(node->child1->getDataType());
				}
			} else {
				analyse(node->child1);
				node->setDataType(node->child1->getDataType());
			}

		} else if (node_type == "logical_expression" || node_type == "and_expression"){
			// Analyse the children
			if(node->child2 != NULL){
				analyse(node->child1);
				analyse(node->child2);
				node->setDataType(dt_bool);
			} else {
				analyse(node->child1);
				node->setDataType(node->child1->getDataType());
			}

		} else if (node_type == "relational_expression" ){
			// Analyse the children
			analyse(node->child1);

			if(node->child3 != NULL){
				analyse(node->child3);
				if(checkDatatypeCoercible(node->child1->getDataType(), node->child3->getDataType())){
					analyse(node->child2);
					node->setDataType(dt_bool);
				} else {
					error_count++;
					error_message<<"Line Number "<<node->line_number<<" : Data type mismatch. Unable to type cast implicitly.(relational_expression)"<<endl;
					node->setDataType(dt_err);
				}
			} else {
				node->setDataType(node->child1->getDataType());
			}

		} else if (node_type == "simple_expression" || node_type == "divmul_expression" ){
			// Analyse the children
			analyse(node->child1);
			if(node->child2 != NULL){
				analyse(node->child3);
				analyse(node->child2);
				if(!checkDatatypeCoercible(node->child1->getDataType(), node->child3->getDataType())){
					error_count++;
					error_message<<"Line Number "<<node->line_number<<" : Data type mismatch. Unable to type cast implicitly.(simple_expression/divmul_expression)"<<endl;
					node->setDataType(dt_err);
				} else {
					DataType dt1 = node->child1->getDataType();
					DataType dt2 = node->child3->getDataType();

					if((dt1 == dt_int) && (dt2 == dt_int)){
						node->setDataType(dt_int);
					} else if((dt1 == dt_int || dt1 == dt_float) && ((dt2 == dt_int || dt2 == dt_float))){
						node->setDataType(dt_float);
					} else {
						error_count++;
						error_message<<"Line Number : "<<node->line_number<<" : Invalid operands provided to '"<<node->child2->getValue()<<"' operator."<<endl;
						node->setDataType(dt_err);
					}
				}
			}else{
				node->setDataType(node->child1->getDataType());
			}

		} else if (node_type == "unary_expression" ){
			// Analyse the children
			if (node->child2 != NULL){
				analyse(node->child2);
				if(node->child2->getDataType() != dt_int && node->child2->getDataType() != dt_float){
						error_count++;
						error_message<<"Line Number : "<<node->line_number<<" : Invalid operands provided to '"<<node->child2->getValue()<<"' unary operator."<<endl;
						node->setDataType(dt_err);
				} else {
					node->setDataType(node->child2->getDataType());
				}
			} else {
				analyse(node->child1);
				node->setDataType(node->child1->getDataType());
			}

		} else if (node_type == "term" ){
			// Analyse the children
			analyse(node->child1);
			node->setDataType(node->child1->getDataType());

		} else if (node_type == "function_call" ){
			// check if function is declared
			node->setDataType(dt_none);

			if(!symtab.find(node->getValue())){
				error_count++;
				error_message<<"Line Number "<<node->line_number<<" : Function '"<< node->getValue() <<"' not declared."<<endl;
				node->setDataType(dt_err);
			} else { // if declared, the arguments count and type should match
				vector<DataType> args_list = expandArgumentsList(node->child1);

				if(!symtab.checkFunctionArgs(node->getValue(), args_list)){
					error_count++;
					error_message<<"Line Number "<<node->line_number<<" : Incorrect arguments passed to the function '"<< node->getValue() <<"'."<<endl;
					node->setDataType(dt_err);
				} else {
					node->setDataType(symtab.getFunctionDataType(node->getValue()));
				}
			}

		} else if (node_type == "operator1" || node_type == "operator2" || node_type == "operator3" || node_type == "unary_operator" || node_type == "constants" ){
			// Analyse the children
			return ;
		} else {
			cout<<"ROOPANSH ABHISHEK"<<endl;
			cout<<node->getValue()<<endl;
			cout<<node->getType()<<endl;
			cout<<node->getDataType()<<endl;
			cout<<"ROOPANSH ABHISHEK"<<endl;
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
		if(tree->getType() != "parameters" || tree->child1 == NULL){
			return vector<Parameter>();
		} else {
			Node *paramlist = tree->child1;
			return expandParameterListAux(paramlist);
		}
	}

	vector<Parameter> expandParameterListAux(Node *tree){
		vector<Parameter> res;
		if(tree->getType() != "parameters_list"){
			return res;
		}
		if(tree->child2 == NULL){
			res.push_back(Parameter(tree->child1->child2->getValue(), tree->child1->getDataType()));
			return res;
		}
		res = expandParameterListAux(tree->child1);
		res.push_back(Parameter(tree->child2->child2->getValue(), tree->child2->getDataType()));
		return res;
	}

	vector<DataType> expandArgumentsList(Node *Tree){
		vector<DataType> v;
		v.clear();
		if(Tree->getType() != "args" || Tree->child1 == NULL){return v;}
		Node *args_list = Tree->child1;
		return expandArgumentsListAux(args_list);
	}

	vector<DataType> expandArgumentsListAux(Node *tree){
		vector<DataType> res;
		res.clear();
		if(tree->getType() != "args_list"){
			return res;
		}
		if(tree->child2 == NULL){
			analyse(tree->child1);
			res.push_back(tree->child1->getDataType());
			return res;
		}
		res = expandArgumentsListAux(tree->child1);
		analyse(tree->child2);
		res.push_back(tree->child2->getDataType());
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

	void errors(){
		if(error_count == 0){
			cout<<"No Semantic Errors!"<<endl;
		} else {
			cout<<error_count<<"  error(s) found during semantic analysis!"<<endl;
			cout<<error_message.str()<<endl;
			exit(2);
		}
	}

	// ~SemanticAnalysis();
};
