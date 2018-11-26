// Cade Schwartz and Seth Parry
// CS 4301
// Compiler Stage 1

#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <string>
#include <sstream>
#include <vector>
#include <cctype>

using namespace std;

const int MAX_SYMBOL_TABLE_SIZE = 256;
enum storeType {INTEGER, BOOLEAN, PROG_NAME};
enum allocation {YES,NO};
enum modes {VARIABLE, CONSTANT};
struct entry //define symbol table entry format
{
	string internalName;
	string externalName;
	storeType dataType;
	modes mode;
	string value;
	allocation alloc;
	int units;
};
vector<entry> symbolTable;
ifstream sourceFile;
ofstream listingFile,objectFile;
string token;
char charac;
const char END_OF_FILE = '$'; // arbitrary choice
int numErrors = 0;
int lineNum = 1;
int numBooleans = 0;
int numIntegers = 0;
int numEntries = 0;

void CreateListingHeader();
void Parser();
void CreateListingTrailer();
void PrintSymbolTable();
void Prog();
void ProgStmt();
void Consts();
void Vars();
void BeginEndStmt();
void ConstStmts();
void VarStmts();
void ExecStmts();
void ExecStmt();
void AssignStmt();
void ReadStmt();
void ReadList();
void WriteStmt();
void WriteList();
storeType Express();
storeType Expresses();
storeType Term();
void Terms();
storeType Factor();
void Factors();
storeType Part();
string RelOp();
string AddLevelOp();
string MultLevelOp();
void Code(string, string = "", string = "");
void PushOperand(string);
string PopOperand();
void PushOperator(string);
string PopOperator();
string Ids();
void Insert(string,storeType, modes, string, allocation, int);
storeType WhichType(string);
string WhichValue(string);
string NextToken();
char NextChar();
void exit(string);
bool isKeyword (string);
bool isNonKeyID (string);
string GenInternalName(storeType);
int getEntryNumber(string);

int main(int argc, char **argv)
{

 //this program is the stage1 compiler for Pascallite. It will accept
 //input from argv[1], generating a listing to argv[2], and object code to
 //argv[3]
 
	sourceFile.open(argv[1]);
    listingFile.open(argv[2]);
    objectFile.open(argv[3]);

	CreateListingHeader();
	Parser();
	CreateListingTrailer();
	PrintSymbolTable();
	
	sourceFile.close();
	listingFile.close();
	objectFile.close();

 return 0;
}

void CreateListingHeader()
{

	time_t timeOfDay = time (NULL);

	//print "STAGE1:", names, DATE, TIME OF DAY;
	listingFile << "STAGE1: Seth Parry, Cade Schwartz"  << setw(32) << ctime(&timeOfDay) << endl;
	//print "LINE NO:", "SOURCE STATEMENT";
	listingFile << "LINE NO." << setw(32) << "SOURCE STATEMENT\n\n";
	//line numbers and source statements should be aligned under the headings
}
void Parser()
{
	NextChar();
	//charac must be initialized to the first character of the source file
	
	if(NextToken() != "program") exit("keyword \"program\" expected");
 
	//a call to NextToken() has two effects
	// (1) the variable, token, is assigned the value of the next token
	// (2) the next token is read from the source file in order to make
	// the assignment. The value returned by NextToken() is also
	// the next token.
	Prog();
	//parser implements the grammar rules, calling first rule
}
void CreateListingTrailer()
{
	//print "COMPILATION TERMINATED", "# ERRORS ENCOUNTERED";
	listingFile << "\nCOMPILATION TERMINATED      " << numErrors << " ERRORS ENCOUNTERED\n";
}

void PrintSymbolTable()
{

	time_t timeOfDay = time (NULL);
	objectFile << "STAGE1: Seth Parry, Cade Schwartz"  << setw(32) << ctime(&timeOfDay) << endl;
	objectFile << "Symbol Table\n\n";
	
	for(unsigned i = 0; i < symbolTable.size(); i++){
		objectFile << left << setw(17) << symbolTable[i].externalName;
		objectFile << left << setw(4) << symbolTable[i].internalName;
		switch(symbolTable[i].dataType){
			case PROG_NAME:
				objectFile << setw(11) << right << "PROG_NAME";
				break;
			case BOOLEAN:
				objectFile << setw(11) << right << "BOOLEAN";
				break;
			case INTEGER:
				objectFile << setw(11) << right << "INTEGER";
		}
		switch(symbolTable[i].mode){
			case CONSTANT:
				objectFile << setw(10) << right << "CONSTANT";
				break;
			case VARIABLE:
				objectFile << setw(10) << right << "VARIABLE";
				break;
		}
		if(symbolTable[i].value == "true")
			objectFile << setw(17) << right << "1";
		else if(symbolTable[i].value == "false")
			objectFile << setw(17) << right << "0";
		else
			objectFile << setw(17) << right << symbolTable[i].value;
		
		switch(symbolTable[i].alloc){
			case YES:
				objectFile << setw(5) << right << "YES";
				objectFile << setw(3) << right << "1";
				break;
			case NO:
				objectFile << setw(5) << right << "NO";
				objectFile << setw(3) << right << "0";
				break;
		}
		objectFile << '\n';
	}

}

void Prog() //token should be "program"
{
	if (token != "program") exit("keyword \"program\" expected.");
	ProgStmt();
	if (token == "const") Consts();
	if (token == "var") Vars();
	if (token != "begin") exit("keyword \"begin\" expected.");
	BeginEndStmt();
	if (token[0] != END_OF_FILE) exit("no text may follow \"end\"");
}

void ProgStmt() //token should be "program"
{
	string x;
	if (token != "program") exit("keyword \"program\" expected.");
	//process error: keyword "program" expected
	x = NextToken();
	if (!isNonKeyID(token)) exit("program name expected");
	//process error: program name expected
	if (NextToken() != ";") exit("semicolon expected");
	//process error: semicolon expected
	NextToken();
	Insert(x,PROG_NAME,CONSTANT,x,NO,0);
}

void Consts() //token should be "const"
{
	if (token != "const") exit("keyword \"const\" expected.");
	//process error: keyword "const" expected
	if (!isNonKeyID(NextToken())) exit("non-keyword identifier must follow \"const\"");
	//process error: non-keyword identifier must follow "const"
	ConstStmts();
}

void Vars() //token should be "var"
{
	if (token != "var") exit("keyword \"var\" expected.");
	//process error: keyword "var" expected
	if (!isNonKeyID(NextToken())) exit("non-keyword identifier must follow \"var\"");
	//process error: non-keyword identifier must follow "var"
	VarStmts();
}

void BeginEndStmt() //token should be "begin"
{
	if (token != "begin") exit("keyword \"begin\" expected.");
	//process error: keyword "begin" expected
	NextToken();
	ExecStmts();
	if (token != "end") exit("keyword \"end\" expected.");
	//process error: keyword "end" expected
	if (NextToken() != ".") exit("period expected.");
	//process error: period expected
	NextToken();
	Code("end", ".");
}

void ConstStmts() //token should be NON_KEY_ID
{
	string x,y;
	if (!isNonKeyID(token)) exit("non-keyword identifier expected");
	//process error: non-keyword identifier expected
	x = token;
	if (NextToken() != "=") exit("\"=\" expected");
	//process error: "=" expected
	y = NextToken();
	if (y != "+" && y != "-" && y != "not" && !isNonKeyID(y) && y != "true" && y != "false" && WhichType(y) != INTEGER) exit("token to right of \"=\" illegal");
	//process error: token to right of "=" illegal
	if (y == "+" || y == "-")
	{
		if(WhichType(NextToken()) != INTEGER) exit("integer expected after sign");
		//if(!isdigit(token[0])) exit("integer expected after sign");
		//process error: integer expected after sign
		y = y + token;
	}
	if (y == "not")
	{
		string t = NextToken();
		if(WhichType(t) != BOOLEAN) exit("boolean expected after not");
		//process error: boolean expected after not
		if(token == "true")
			y = "false";
		else
			y = "true";
	}
	if (NextToken() != ";") exit("semicolon expected");
	//process error: semicolon expected
	if(WhichType(y) == PROG_NAME) exit("data type must be INTEGER or BOOLEAN");
	Insert(x,WhichType(y),CONSTANT,WhichValue(y),YES,1);
	string nextToken = NextToken();
	if (nextToken != "begin" && nextToken != "var" && !isNonKeyID(nextToken)) exit("non-keyword identifier,\"begin\", or \"var\" expected");
	//process error: non-keyword identifier,"begin", or "var" expected
	if (isNonKeyID(nextToken))
		ConstStmts();
}

void VarStmts() //token should be NON_KEY_ID
{
	string x,y;
	if (!isNonKeyID(token)) exit("non-keyword identifier expected");
	//process error: non-keyword identifier expected
	x = Ids();
	if (token != ":") exit("\":\" expected");
	//process error: ":" expected
	string nextToken = NextToken();
	if(nextToken != "integer" && nextToken != "boolean") exit("illegal type follows \":\"");
	//process error: illegal type follows ":"
	y = nextToken;
	if(NextToken() != ";") exit("semicolon expected");
	//process error: semicolon expected
	if(WhichType(y) == PROG_NAME) exit("BOOLEAN or INTEGER expected");
	Insert(x,WhichType(y),VARIABLE,"",YES,1);
	nextToken = NextToken();
	if (nextToken != "begin" && !isNonKeyID(nextToken)) exit("non-keyword identifier or \"begin\" expected");
	//process error: non-keyword identifier or "begin" expected
	if (isNonKeyID(nextToken)) VarStmts();
}

void ExecStmts()
{
	string nextToken = token;
	if(!isNonKeyID(token) && token != "read" && token != "write" && token != "end") exit("non-keyword identifier, 'read', 'write', or 'end' expected");
	if(token != "end"){
		ExecStmt();
		string nextToken = NextToken();
	}
	if(isNonKeyID(nextToken) || nextToken == "read" || nextToken == "write") ExecStmts();
}

void ExecStmt()
{
	if(!isNonKeyID(token) && token != "read" && token != "write" && token != "end") exit("non-keyword identifier, 'read', 'write', or 'end' expected");
	if(token == "read") ReadStmt();
	if(token == "write") WriteStmt();
	if(isNonKeyID(token)){
		if(symbolTable[getEntryNumber(token)].mode != VARIABLE) exit("identifier to the left of ':=' must be a variable");
		AssignStmt();
	}
}

void AssignStmt()
{
	storeType idType;
	if(!isNonKeyID(token)) exit("non-keyword id expected");
	string name = token;
	if (name.length() > 15) {
				name = name.substr(0, 15);
			}
	idType = WhichType(name);
	PushOperand(name);
	if(NextToken() != ":=") exit("\":=\" expected");
	NextToken();
	PushOperator(":=");
	storeType expressType = Express();
	if(idType != expressType){
		if(idType == BOOLEAN) exit("can't set boolean to int value");
		if(idType == INTEGER) exit("can't set int to boolean value");
	}
	if(token != ";") exit("\";\" expected");
	Code(PopOperator(), PopOperand(), PopOperand());
}

void ReadStmt()
{
	if(token != "read") exit("'read' expected");
	NextToken();
	ReadList();
	if(NextToken() != ";") exit("\";\" expected");
}

void ReadList()
{
	if(token != "(") exit("'(' expected");
	NextToken();
	string x = Ids();
	string name;
	string::iterator itr;
		for (itr = x.begin(); itr <= x.end(); itr += 1) { 
		if(*itr == ',' || itr == x.end()) {
			if (name.length() > 15) {
				name = name.substr(0, 15);
			}

			if (getEntryNumber(name) == -1) exit("reference to undefined variable '" + name + "'");
			if(symbolTable[getEntryNumber(name)].mode != VARIABLE) exit("reading in of read-only location '" + name + "'");
			
			if (isKeyword(name)) exit("illegal use of keyword");
			
			Code("read", name);
			name = "";
		} 
		else {
			name += *itr;
		}		
	} 
	if(token != ")") exit("',' or ')' expected after non-keyword identifier");
}

void WriteStmt()
{
	if(token != "write") exit("'write' expected");
	NextToken();
	WriteList();
	if(NextToken() != ";") exit("\";\" expected");
}

void WriteList()
{
	if(token != "(") exit("'(' expected");
	NextToken();
	string x = Ids();
	string name;
	string::iterator itr;
		for (itr = x.begin(); itr <= x.end(); itr += 1) { 
		if(*itr == ',' || itr == x.end()) {
			if (name.length() > 15) {
				name = name.substr(0, 15);
			}

			if (getEntryNumber(name) == -1) exit("reference to undefined variable '" + name +"'");
			
			if (isKeyword(name)) exit("illegal use of keyword");
			
			Code("write", name);
			name = "";
		} 
		else {
			name += *itr;
		}		
	} 
	if(token != ")") exit("',' or ')' expected after non-keyword identifier");
}

storeType Express()
{
	string name = token;
	if (name.length() > 15) {
				name = name.substr(0, 15);
	}
	if(!isNonKeyID(token) && token != "not" && token != "true" && token != "false"
			&& token != "(" && token != "+" && token != "-"	&& WhichType(name) != INTEGER)
		exit("Invalid token");
	storeType termType = Term();
	storeType expressType = Expresses();
	if(expressType == BOOLEAN) return BOOLEAN;
	else return termType;
}

storeType Expresses()
{
	string tempToken = token;
	if(token == "<>" || token == "=" || token == "<=" || token == ">=" || token == "<" || token == ">"){
		PushOperator(RelOp());
		Term();
		Code(PopOperator(), PopOperand(), PopOperand());
		Expresses();
		return BOOLEAN;
	}
	else if(token != ")" && token != ";") exit("';' expected");
		return PROG_NAME;
}

storeType Term()
{
	string name = token;
	if (name.length() > 15) {
				name = name.substr(0, 15);
	}
	if(!isNonKeyID(token) && token != "not" && token != "true" && token != "false"
			&& token != "(" && token != "+" && token != "-"	&& WhichType(name) != INTEGER)
		exit("Invalid token");
	storeType temp = Factor();
	Terms();
	return temp;
}

void Terms()
{
	string tempToken = token;
	if(token == "-" || token == "+" || token == "or"){
		PushOperator(AddLevelOp());
		PushOperator(x)
		storeType temp = Factor();
		if(tempToken == "+" && temp != INTEGER) exit("operator + requires integer operands");
		if(tempToken == "-" && temp != INTEGER) exit("operator - requires integer operands");
		Code(PopOperator(), PopOperand(), PopOperand());
		Terms();
	}
	else if(token != "<>" && token != "=" && token != "<=" && token != ">=" && token != "<" && token != ">"
			&& token != ")" && token != ";" && token != "-" && token != "+" && token != "or")
		exit("Invalid token");
}

storeType Factor()
{
	string name = token;
	if (name.length() > 15) {
				name = name.substr(0, 15);
	}
	if(token != "not" && token != "true" && token != "false" && token != "(" && token != "+" && token != "-" && !isNonKeyID(token) && WhichType(name) != INTEGER)
		exit("Invalid token");
	storeType temp = Part();
	Factors();
	return temp;
}

void Factors()
{
	string tempToken = token;
	if(token == "*" || token == "div" || token == "mod" || token == "and"){
		PushOperator(MultLevelOp());
		storeType temp = Part();
		if(tempToken == "and" && temp != BOOLEAN) exit("operator and requires boolean operands");
		else if(tempToken == "*" &&temp != INTEGER) exit("operator * requires integer operands");
		else if(tempToken == "div" &&temp != INTEGER) exit("operator div requires integer operands");
		else if(tempToken == "mod" &&temp != INTEGER) exit("operator mod requires integer operands");
		Code(PopOperator(), PopOperand(), PopOperand());
		Factors();
	}
	else if(token != "<>" && token != "=" && token != "<=" && token != ">=" && token != "<" && token != ">" && token != ")" && token != ";" && token != "-" && token != "+" && token != "or")
		exit("invalid operation");
}

storeType Part()
{
	string name = token;
	if (name.length() > 15) {
				name = name.substr(0, 15);
	}
	if(token == "not"){
		NextToken();
		name = token;
		if(token == "("){
			NextToken();
			storeType temp = Express();
			if(token != ")") exit ("\")\" expected");
			NextToken();
			Code("not", PopOperand());
			if(temp != BOOLEAN) exit("not must be followed by boolean");
			return BOOLEAN;
		}
		else if(token == "true" || token == "false"){
			PushOperand("not " + token);
			NextToken();
			return BOOLEAN;
		}
		else if(isNonKeyID(token)){
			if(WhichType(name) != BOOLEAN) exit("not must be followed by boolean");
			Code("not", name);
			NextToken();
			return BOOLEAN;
		}
	}
	else if(token == "+"){
		NextToken();
		name = token;
		if(token == "("){
			NextToken();
			storeType temp = Express();
			if(token != ")") exit ("\")\" expected");
			Code("neg", PopOperand());
			NextToken();
			if(temp != INTEGER) exit("unary + must be followed by integer");
			return INTEGER;
		}
		else if(WhichType(name) == INTEGER){
			PushOperand(token);
			NextToken();
			return INTEGER;
		}
		else if(isNonKeyID(token)){
			if(WhichType(name) != INTEGER) exit("unary + must be followed by integer");
			PushOperand(token);
			NextToken();
			return INTEGER;
		}
	}
	else if(token == "-"){
		NextToken();
		name = token;
		if(token == "("){
			NextToken();
			storeType temp = Express();
			if(token != ")") exit ("\")\" expected");
			Code("neg", PopOperand());
			NextToken();
			if(temp != INTEGER) exit("unary - must be followed by integer");
			return INTEGER;
		}
		else if(WhichType(name) == INTEGER){
			PushOperand("-" + token);
			NextToken();
			return INTEGER;
		}
		else if(isNonKeyID(token)){
			if(WhichType(name) != INTEGER) exit("unary - must be followed by integer");
			Code("neg", token);
			NextToken();
			return INTEGER;
		}
	}
	else if(token == "("){
		NextToken();
		storeType temp = Express();
		if(token != ")") exit ("\")\" expected");
		NextToken();
		return temp;
	}
	else if(WhichType(name) == INTEGER){
		PushOperand(token);
		NextToken();
		return INTEGER;
	}
	else if(token == "true" || token == "false"){
		PushOperand(token);
		NextToken();
		return BOOLEAN;
	}
	else if(isNonKeyID(token)){
		PushOperand(token);
		storeType temp = WhichType(name);
		NextToken();
		return temp;
	}
	else exit("invalid token");
}

string RelOp(){
	if(token == "="){
		NextToken();
		return "=";
	}
	else if(token == "<>"){
		NextToken();
		return "<>";
	}
	else if(token == "<="){
		NextToken();
		return "<=";
	}
	else if(token == ">="){
		NextToken();
		return ">=";
	}
	else if(token == "<"){
		NextToken();
		return "<";
	}
	else if(token == ">"){
		NextToken();
		return ">";
	}
	else exit("Invalid operator");
}

string AddLevelOp(){
	if(token == "+"){
		NextToken();
		return "+";
	}
	else if(token == "-"){
		NextToken();
		return "-";
	}
	else if(token == "or"){
		NextToken();
		return "or";
	}
	else exit("invalid operator");
}

string MultLevelOp(){
	if(token == "*"){
		NextToken();
		return "*";
	}
	else if(token == "div"){
		NextToken();
		return "div";
	}
	else if(token == "mod"){
		NextToken();
		return "mod";
	}
	else if(token == "and"){
		NextToken();
		return "and";
	}
	else exit("invalid operator");
}

string Ids() //token should be NON_KEY_ID
{
 string temp,tempString;
	if (!isNonKeyID(token)) exit("non-keyword identifier expected");
	//process error: non-keyword identifier expected
	tempString = token;
	temp = token;
	if(NextToken() == ",")
	{
		string newtemp = NextToken();
		if (isKeyword(newtemp)) exit("non-keyword identifier expected");
		//process error: non-keyword identifier expected
		tempString = temp + "," + Ids();
	}
	return tempString;
}

void Insert (string externalName,storeType inType, modes inMode, string inValue, allocation inAlloc, int inUnits) {
 //create symbol table entry for each identifier in list of external names
 //Multiply inserted names are illegal
	string::iterator itr;
	string name = "";
	entry e;
	e.dataType = inType;
	e.mode = inMode;
	e.value = inValue;
	e.alloc = inAlloc;
	e.units = inUnits;	
	
	for (itr = externalName.begin(); itr <= externalName.end(); itr += 1) { // make sure that name is not empty or at the end
		if(e.dataType == PROG_NAME) {
			e.value = name.substr(0, 15);
		}
	 
		if(*itr == ',' || itr == externalName.end()) {
			if (name.length() > 15) {
				name = name.substr(0, 15);
			}

			if (getEntryNumber(name) != -1) exit("multiple name definition");
			
			if (isKeyword(name)) exit("illegal use of keyword");
			
			else {
				if (isupper(name[0])) {
					e.externalName = name;
					e.internalName = name;
					symbolTable.push_back(e);
				}
				else {
					e.externalName = name;
					e.internalName = GenInternalName(inType);
					symbolTable.push_back(e);
				}
				numEntries++;
				if(numEntries > MAX_SYMBOL_TABLE_SIZE) exit("Symbol table is overflowed");
			}
			name = "";
		} 
		else {
			name += *itr;
		}		
	} 
}


storeType WhichType(string name) //tells which data type a name has
{
	storeType type = PROG_NAME;
	if (name == "true" || name == "false" || name == "not true" || name == "not false" || name == "boolean") {//name is a boolean literal then data type = BOOLEAN
		type = BOOLEAN;
		return type;
	}
	else if (isdigit(name[0]) || name[0] == '+' || name[0] == '-' || name == "integer") { //data type = INTEGER
		type = INTEGER;
		return type;
	}
	else if(getEntryNumber(name) != -1) { //name is an identifier and hopefully a constant
		//if symbolTable[name] is defined then data type = type of symbolTable[name]
		return symbolTable[getEntryNumber(name)].dataType;
	}
	else exit("reference to undefined constant"); //process error: reference to undefined constant
	return type;
}

string WhichValue(string name) //tells which value a name has
{
	string value;
	if (WhichType(name) == BOOLEAN || isdigit(name[0]) || name[0] == '+' || name[0] == '-') //name is a literal
		value = name;
	else if(getEntryNumber(name) != -1 && symbolTable[getEntryNumber(name)].value != " ")//name is an identifier and hopefully a constant
	//if symbolTable[name] is defined and has a value
		value = symbolTable[getEntryNumber(name)].value;
	else exit("reference to undefined constant");
	//process error: reference to undefined constant
	return value;
}

string NextToken() //returns the next token or end of file marker
{
	token = "";
	while (token == "") {
		if(charac == '{') {
			while (charac != END_OF_FILE && charac != '}'){
				if(charac == END_OF_FILE) exit("unexpected end of file");
				//process error: unexpected end of file
				else 
					NextChar();
			}
			NextChar();
		}
		else if(charac == '}'){
			exit("'}' cannot begin token");
		}
		else if(isspace(charac)){
			NextChar();
		}
		else if (charac == '=' || charac == ',' || charac == ';' || charac == '.' || charac == '+' || charac == '-' || charac == '(' || charac == ')' || charac == '*'){
			token = charac;
			NextChar();
		}
		else if (charac == '<'){
			token = charac;
			if(sourceFile.peek() == '=' || sourceFile.peek() == '>')
				token += NextChar();
			NextChar();
		}
		else if (charac == ':' || charac == '>'){
			token = charac;
			if(sourceFile.peek() == '=')
				token += NextChar();
			NextChar();
		}		
		else if(islower(charac)) {
			token = charac;
			while(isalpha(NextChar()) || isdigit(charac) || charac == '_'){
				if(token.back() == '_' && charac == '_') exit("_ cannot follow _");
				token += charac;	
			}
			if(token.back() == '_') exit("'_' cannot end token");
		}
		else if(isdigit(charac)) {
			token = charac;
			while(isdigit(NextChar()))
				token += charac;
		}
		else if(charac == END_OF_FILE) {
			token = charac;
		}
		else exit("illegal symbol");
		
	}
	return token;
}

char NextChar() //returns the next character or end of file marker
{
	char next = END_OF_FILE;
	static bool newLine = true;
	sourceFile.get(next);
	//read in next character
	if (next == END_OF_FILE)
		charac = END_OF_FILE; //use a special character to designate end of file
	else
		charac = next;
		
	//print to listing file (starting new line if necessary)
	if (newLine && charac != END_OF_FILE) {
		listingFile << setw(5) << lineNum << "|";
		newLine = false;
	}
	
	if(charac == '\n'){
		lineNum++;
		newLine = true;
	}
	
	if(charac != END_OF_FILE)
		listingFile << charac;
 
	return charac;
}

bool isKeyword (string x) {
	if (x == "program" || x == "begin" || x == "end" || x == "var" || x == "const" 
				|| x == "integer" || x == "boolean" || x == "true" || x == "false" 
				|| x == "not" || x == "mod" || x == "div" || x == "and" || x == "or"
				|| x == "read" || x == "write") 
		return true;
	else 
		return false;
}

bool isNonKeyID (string x) {
	if(islower(x[0]) && !isKeyword(x)) {
		for(unsigned i = 1; i < x.length(); i += 1) {
			if(!isdigit(x[i]) && !islower(x[i]) && x[i] != '_') {
				return false;
			}
		} 
		return true;
	}
	return false;
}

void exit(string s) {
	listingFile << "\nError: Line " << lineNum << ": " << s << endl;
	sourceFile.close();
	numErrors++;
	CreateListingTrailer();
	listingFile.close();
	objectFile.close();
	exit(-1);
}

string GenInternalName(storeType x){
	string output;
	if(x == PROG_NAME)
		output = "P0";
	else if(x == INTEGER){
		output = "I" + to_string(numIntegers);
		numIntegers++;
	}
	else if(x == BOOLEAN) {
		output = "B" + to_string(numBooleans);
		numBooleans++;
	}
	return output;
}

int getEntryNumber(string name){
	for(unsigned i = 0; i < symbolTable.size(); i++){
		if(symbolTable[i].externalName == name){
			return i;
		}
		else if(symbolTable[i].internalName == name){
			return i;
		}
	}
	return -1;
}

void Code(string op, string operand1, string operand2)
{
 if (op == "program"); //emit first RAMM instruction STRT NOP …
 else if (op == "end"); //emit HLT, BSS and DEC pseudo ops, and END
 else if (op == "read"); //emit read code;
 else if (op == "write"); //emit write code
 else if (op == "+"); //emit addition code; //this must be binary '+'
 else if (op == "-"); //emit subtraction code; //this must be binary '-'
 else if (op == "neg"); //emit negation code; //this must be unary '-'
 else if (op == "not"); //emit not code;
 else if (op == "*"); //emit multiplication code;
 else if (op == "div"); //emit division code;
 else if (op == "mod"); //emit modulo code;
 else if (op == "and"); //emit and code;
 else if (op == "="); //emit equality code;
 else if (op == ":="); //emit assignment code;
 else exit("undefined operation"); //process error: undefined operation
 }


void PushOperator(string name) //push name onto operatorStk
{
 //push name onto stack;
}

void PushOperand(string name) //push name onto operandStk
 //if name is a literal, also create a symbol table entry for it
{
 //if name is a literal and has no symbol table entry
 //insert symbol table entry, call whichType to determine the data type of the literal
 //push name onto stack;
}

string PopOperator() //pop name from operatorStk
{
 //if operatorStk is not empty
 //return top element removed from stack;
 //else process error: operator stack underflow;
 return "";
}

string PopOperand() //pop name from operandStk
{
 //if operandStk is not empty
 //return top element removed from stack;
 //else process error: operand stack underflow;
}