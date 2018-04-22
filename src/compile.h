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

	string TYPE2STRING[6] = {"none", "int", "float", "bool", "func", "err"};


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
			if(symbols[i].find(id) !=  symbols[i].end()){
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
			if(symbols[i].find(id) !=  symbols[i].end()){
				return (symbols[i].find(id))->second.getReturnDataType();
			}
		}
		return dt_none;
	}

	bool checkFunctionArgs(string id, vector<DataType> args_list) {
		for (int i = scope; i >= 0; i--)
		{
			if(symbols[i].find(id) !=  symbols[i].end()){
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

	vector<string> getFunctionParameters(string id){
		for (int i = scope; i >= 0; i--)
		{
			if(symbols[i].find(id) !=  symbols[i].end()){
				vector<Parameter> param_list =  symbols[i].find(id)->second.getParameterList();
				vector<string> res;
				for (std::vector<Parameter>::iterator i = param_list.begin(); i != param_list.end(); ++i)
				{
					res.push_back(i->getValue());
				}
				return res;
			}
		}
	}

	string gen_mips(string id){
		// return name.datatype.scope
		for (int i = scope; i >= 0; i--) {
			if(symbols[i].find(id) !=  symbols[scope].end()){
				return id + "." +  TYPE2STRING[symbols[i].find(id)->second.getDataType()] + "." + to_string(i);
			}
		}
		return "";
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
			if(node->getValue() == "break"){
				if (inside_loop){
					return ;
				} else {
					error_count++;
					error_message << "Line Number " << node->line_number << " : 'break' can only be used inside a loop." << endl;
				}
			} else if (node->getValue() == "continue"){
				if (inside_loop){
					return ;
				} else {
					error_count++;
					error_message << "Line Number " << node->line_number << " : 'continue' can only be used inside a loop." << endl;
				}
			} else {
				analyse(node->child1);
			}

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
			inside_loop = true;
			analyse(node->child1);
			inside_loop = false;

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


class MIPSCode
{
private:
	SymbolTable symtab, backup_symtab;	// backup symtab is used for pushing to stack during backup
	stringstream mips_1, mips_2;
	int label_counter;
	int temp_counter;

	vector<string> breaks;
	vector<string> continues;

	vector<string> allVariables;

	string TYPE2STRING[6] = {"none", "int", "float", "bool", "func", "err"};

public:
	MIPSCode(){
		mips_1 << ".text" << endl;
		label_counter = 0;
		temp_counter = 0;

		breaks.clear();
		continues.clear();
		allVariables.clear();
	}

	string getNextLabel(){
		return "label_" + to_string(label_counter++);
	}

	string getNextTempVar(){
		return "temp" + to_string(temp_counter++);
	}

	string putLabel(string label, int param_count){
		label = "_" + label + "_." + to_string(param_count);
		mips_1 << label << " : " << endl;
		return label;
	}

	string putLabel(string label){
		mips_1 << label << " : " << endl;
		return label;
	}

	void loadInRegister(string var, string reg){
		// check if var is variable (load word) or number (load immediate)

		if(var[0] <= '9' and var[0] >= '0')	// if number
			mips_1 << "li\t$" << reg <<", " << var << endl;
		else					// variable name
		{
			var = "_" + var;
			mips_1 << "lw\t$" << reg <<", " << var << endl;
			if(std::find(allVariables.begin(), allVariables.end(), var) == allVariables.end())
				allVariables.push_back(var);
		}
	}

	void storeInMemory(string var){
		var = "_"+var;
		mips_1 << "sw\t$t0, " << var << endl;
		if(std::find(allVariables.begin(), allVariables.end(), var) == allVariables.end())
			allVariables.push_back(var);
	}

	void ReturnFunc(){
		mips_1 << "jr\t$ra"<<endl;
	}

	void Jump(string label){
		mips_1 << "j\t" << label << endl;
	}

	void condition(string reg, string label){
		mips_1 << "bgtz\t$" << reg <<", "<<label<<endl;
	}

	void functionReturnValue(string reg){
		mips_1 << "move\t$v0, $"<< reg << endl;
	}

	void readCode(string reg){
		mips_1 << "li\t$v0, 5" << endl;
		mips_1 << "syscall" << endl;
		mips_1 << "sw\t$v0, $" << reg << endl;
	}

	void writeCode(string reg){
		mips_1 << "li\t$v0, 1" << endl;
		mips_1 << "move\t$a0, $"<< reg << endl;
		mips_1 << "syscall" << endl;
	}

	void pushReturnCode(){
		mips_1 << "addi\t$sp,$sp,-4" << endl;
		mips_1 << "sw\t$ra,0($sp)" << endl;
	}

	void popReturnCode(){
		mips_1 << "addi\t$sp,$sp,-4" << endl;
		mips_1 << "sw\t$ra,0($sp)" << endl;
	}

	void copy(string src, string dst){
		mips_1 << "lw\t$t8, " << src << endl;
		mips_1 << "sw\t$t8, " << dst << endl;
	}

	void functionCall(string fun){
		mips_1 << "jal\t" << fun << endl;
	}

	void restoreReturn(string ret){
		mips_1 << "sw\t$v0, " << ret << endl;
	}

	void operate(string op){
		if(op == "&&") {
			mips_1 << "and\t$t0, $t1, $t2" << endl;
		} else if(op == "||") {
			mips_1 << "or\t$t0, $t1, $t2" << endl;

		} else if(op == "+") {
			mips_1 << "add\t$t0, $t1, $t2" << endl;
		} else if(op == "-") {
			mips_1 << "sub\t$t0, $t1, $t2" << endl;

		} else if(op == "*") {
			mips_1 << "mult\t$t1, $t2" << endl;		// store the result in $LO
			mips_1 << "mflo\t$t0" << endl;			// load the contents of $LO to $t0
		} else if(op == "/") {
			mips_1 << "div\t$t1, $t2" << endl;
			mips_1 << "mflo\t$t0" << endl;

		} else if(op == ">=") {
			mips_1 << "slt\t$t3, $t1, $t2" << endl;		// set t3 if t1 less than t2
			mips_1 << "xori\t$t0, $t3, 1" << endl;		// compliment t3
		} else if(op == "<=") {
			mips_1 << "slt\t$t3, $t2, $t1" << endl;		// set t3 if t2 less than t1
			mips_1 << "xori\t$t0, $t3, 1" << endl;		// compliment t3

		} else if(op == ">") {
			mips_1 << "slt\t$t0, $t2, $t1" << endl;		// set t0 if t2 less than t1
		} else if(op == "<") {
			mips_1 << "slt\t$t0, $t1, $t2" << endl;

		} else if(op == "==") {
			mips_1 << "slt\t$t3, $t1, $t2" << endl;
			mips_1 << "slt\t$t4, $t2, $t1" << endl;
			mips_1 << "or\t$t5, $t3, $t4" << endl;
			mips_1 << "xori\t$t0, $t5, 1" << endl;
		} else if(op == "!=") {
			mips_1 << "slt\t$t3, $t1, $t2" << endl;
			mips_1 << "slt\t$t4, $t2, $t1" << endl;
			mips_1 << "or\t$t0, $t3, $t4" << endl;
		} else {
			cerr<<"WRONG OPERATOR PASSED!";
		}
	}

	string generateCode(Node *tree){
		if(tree == NULL)	return "";

		string node_type = tree->getType();

		DEBUG cerr << node_type << endl;

		if (node_type == "variable_declaration") {
			if(tree->child3 == NULL){
				vector<string> vars = expandVariablesList(tree->child2);
				for(int i=0; i < vars.size(); i++)
				{
					symtab.addVariableInCurrentScope(vars[i], tree->child1->getDataType());
				}
			} else {
				cerr<<"HERE"<<endl;
				// add to symtab
				symtab.addVariableInCurrentScope(tree->child2->getValue(), tree->child1->getDataType());
				string exp = generateCode(tree->child3);
				// store the expression register to the memory
				loadInRegister(exp, "t0");
				storeInMemory(symtab.gen_mips(tree->child2->getValue()));
			}
			return "";

		} else if ( node_type == "variable") {
			// return name.type.scope
			return symtab.gen_mips(tree->child1->getValue());

		} else if ( node_type == "function_declaration") {

			vector<Parameter> params = expandParameterList(tree->child2->child1);

			string label = putLabel(tree->getValue(), params.size());

			symtab.addFunction(label, dt_int, params);
			symtab.addScope();

			for(int i=0;i<params.size();i++)
			{
				symtab.addVariableInCurrentScope(params[i].getValue(), params[i].getDataType());
			}

			string a = generateCode(tree->child3);
			symtab.removeScope();

			ReturnFunc();

			return "";

		} else if ( node_type == "main_function") {
			mips_1 << "main : "<<endl;
			symtab.addScope();
			string a = generateCode(tree->child1);
			symtab.removeScope();
			ReturnFunc();
			return "";

		} else if ( node_type == "statement") {
			if(tree->getValue() == "break"){
				Jump(breaks.back());
			} else if (tree->getValue() == "continue") {
				Jump(continues.back());
			} else {
				generateCode(tree->child1);
			}
			return "";

		} else if ( node_type == "condition") {
			string start = getNextLabel();
			string end = getNextLabel();

			string a = generateCode(tree->child1);	// expression
			// load the expression's value in t1
			loadInRegister(a, "t1");
			// branch to start if $t1>0
			condition("t1", start);
			// otherwise go to end
			Jump(end);
			// now add the if code (i.e. the start label)
			putLabel(start);
			symtab.addScope();
			string b = generateCode(tree->child2);
			symtab.removeScope();
			putLabel(end);

			// if else part
			if(tree->child3 != NULL)
			{
				symtab.addScope();
				string c = generateCode(tree->child3);
				symtab.removeScope();
			}
			return "";

		} else if ( node_type == "for_loop") {
			string start = getNextLabel();		// start of the loop
			string middle = getNextLabel();		// code statements
			string con = getNextLabel();		// jump to this label on continue/condition reevaluate
			string end = getNextLabel();		// end

			string a = generateCode(tree->child1);		// variable
			string b = generateCode(tree->child2);		// expression


			breaks.push_back(end);		// Label to jump to on break
			continues.push_back(con); 	// Label to jump to on continue

			// start variables value from 0
			loadInRegister("0", "t1");
			loadInRegister("0", "t2");
			operate("+");	// add t1 & t2 and store to t0
			storeInMemory(a);	//store a to temp

			// start of the loop
			putLabel(start);

			// check the condition - variable is less than the simple_expression value
			// set t0 = 1 if a < b
			string temp = getNextTempVar();
			loadInRegister(a, "t1");
			loadInRegister(b, "t2");
			operate("<");
			storeInMemory(temp);	// store t0 to temp

			// if variable in expression, then jump to the code
			loadInRegister(temp, "t1");
			condition("t1", middle);
			// otherwise jump to end
			Jump(end);

			putLabel(middle);
			symtab.addScope();
			string c = generateCode(tree->child3);
			symtab.removeScope();

			putLabel(con);

			loadInRegister(a, "t1");
			loadInRegister("1", "t2");
			operate("+");	// add t1 & t2 and store to t0
			storeInMemory(a);	//store a to temp

			Jump(start);
			putLabel(end);

			breaks.pop_back();
			continues.pop_back();
			return "";

		} else if ( node_type == "while_loop") {
			string start = getNextLabel();	// start of the loop
			string middle = getNextLabel();		// loop statements
			string end = getNextLabel();		// terminate loop

			breaks.push_back(end);
			continues.push_back(start);

			putLabel(start);
			string a = generateCode(tree->child1);

			loadInRegister(a, "t1");
			condition("t1", middle);
			Jump(end);
			putLabel(middle);

			symtab.addScope();
			string b = generateCode(tree->child2);
			symtab.removeScope();

			Jump(start);
			putLabel(end);

			breaks.pop_back();
			continues.pop_back();

			return "";

		} else if ( node_type == "return_statement") {
			if(tree->child2 == NULL)
			{
				string a = generateCode(tree->child2);

				loadInRegister(a, "t1");
				functionReturnValue("t1");
			}
			ReturnFunc();

			return "";

		} else if ( node_type == "read") {
			string a = generateCode(tree->child1);
			loadInRegister(a, "t1");
			readCode("t1");
			return a;

		} else if ( node_type == "write") {
			string a = generateCode(tree->child1);
			loadInRegister(a, "t1");
			writeCode("t1");
			return a;

		} else if ( node_type == "expression") {
			string b;
			if(tree->getValue() == "=") {
				string a = generateCode(tree->child2);
				b = generateCode(tree->child1);
				loadInRegister(a, "t1");
				loadInRegister("0", "t2");
				operate("+");
				storeInMemory(b);
			} else {
				b = generateCode(tree->child1);
			}
			string ret = getNextTempVar();

			loadInRegister(b, "t1");
			loadInRegister("0", "t2");
			operate("+");
			storeInMemory(ret);
			return ret;

		} else if ( node_type == "logical_expression") {
			if(tree->getValue() == "or") {
				string a = generateCode(tree->child2);
				string b = generateCode(tree->child1);
				string ret = getNextTempVar();

				loadInRegister(b, "t1");
				loadInRegister(a, "t2");
				operate("||");
				storeInMemory(ret);
				return ret;

			} else {
				return generateCode(tree->child1);
			}

		} else if ( node_type == "and_expression") {
			if(tree->getValue() == "and") {
				string a = generateCode(tree->child2);
				string b = generateCode(tree->child1);
				string ret = getNextTempVar();

				loadInRegister(b, "t1");
				loadInRegister(a, "t2");
				operate("&&");
				storeInMemory(ret);
				return ret;

			} else {
				return generateCode(tree->child1);
			}

		} else if ( node_type == "relational_expression") {
			if(tree->getValue() == "op")
			{
				string a = generateCode(tree->child3);
				string b = generateCode(tree->child1);
				string c = generateCode(tree->child2);
				string ret = getNextTempVar();

				loadInRegister(b, "t1");
				loadInRegister(a, "t2");
				operate(c);
				storeInMemory(ret);
				return ret;
			} else {
				return generateCode(tree->child1);
			}

		} else if ( node_type == "simple_expression") {
			if(tree->getValue() == "op")
			{
				string a = generateCode(tree->child3);
				string b = generateCode(tree->child1);
				string c = generateCode(tree->child2);
				string ret = getNextTempVar();

				loadInRegister(b, "t1");
				loadInRegister(a, "t2");
				operate(c);
				storeInMemory(ret);
				return ret;
			} else {
				return generateCode(tree->child1);
			}

		} else if ( node_type == "divmul_expression") {
			if(tree->getValue() == "op")
			{
				string a = generateCode(tree->child3);
				string b = generateCode(tree->child1);
				string c = generateCode(tree->child2);
				string ret = getNextTempVar();

				loadInRegister(b, "t1");
				loadInRegister(a, "t2");
				operate(c);
				storeInMemory(ret);
				return ret;
			} else {
				return generateCode(tree->child1);
			}

		} else if ( node_type == "unary_expression") {
			if(tree->getValue() == "op") {
				string a = generateCode(tree->child2);
				string b = generateCode(tree->child1);
				string ret = getNextTempVar();

				loadInRegister("0", "t1");
				loadInRegister(a, "t2");
				operate(b);
				storeInMemory(ret);
				return ret;
			} else {
				return generateCode(tree->child1);
			}

		} else if ( node_type == "term") {
			return generateCode(tree->child1);

		} else if ( node_type == "constants") {
			return tree->getValue();

		} else if ( node_type == "function_call") {
			//evaluate arguments

			vector<string> args = expandArgumentsList(tree->child1);
			vector<string> pars = symtab.getFunctionParameters("_" + tree->getValue() + "_." + to_string(args.size()));

			//backup variables
			// push the return address to stack
			pushReturnCode();

			// push the current register values to stack
			// vector<string> backvars = mipstable.backup();
			// for(int i=0;i<backvars.size();i++)
			// {
			// 	genmips(def,"push",backvars[i]);
			// }

			//set arguments

			// copy args[i] to pars[i]
			for(int i=0;i < args.size(); i++)
			{
				copy(args[i], pars[i]);
			}

			//call function
			functionCall("_" + tree->getValue() + "_." + to_string(args.size()));

			//restore variables
			// pop back the stored register values from stack
			// for(int i = backvars.size()-1;i>=0;i--)
			// {
			// 	genmips(def,"pop",backvars[i]);
			// }
			// pop back the return address from stack
			// mipstable.restore(backvars);

			popReturnCode();

			string ret = getNextTempVar();

			// restore the return value
			restoreReturn(ret);

			return ret;

		} else if ( node_type == "unary_operator" || node_type == "operator3" || node_type == "operator2" || node_type == "operator1") {
			return tree->getValue();

		} else {
			generateCode(tree->child1);
			generateCode(tree->child2);
			generateCode(tree->child3);
			return "";
		}
	}

	void generateDataSection(){
		return ;
	}

	void generateOutput(){
		cout<<mips_1.str()<<endl;
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

	vector<string> expandArgumentsList(Node *Tree){
		vector<string> v;
		v.clear();
		if(Tree->getType() != "args" || Tree->child1 == NULL) {return v;}
		Node *args_list = Tree->child1;
		return expandArgumentsListAux(args_list);
	}

	vector<string> expandArgumentsListAux(Node *tree){
		vector<string> res;
		res.clear();
		if(tree->getType() != "args_list"){
			return res;
		}
		if(tree->child2 == NULL){
			res.push_back(tree->child1->getValue());
			return res;
		}

		res = expandArgumentsListAux(tree->child1);
		res.push_back(tree->child2->getValue());

		return res;
	}


	// ~MIPSCode();
};
