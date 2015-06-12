/*
= Brainfuck

If you have gcc:

----
g++ -o brainfuck.exe brainfuck.cpp
brainfuck.exe helloworld.bf
----
*/

#include <vector>
#include <iostream>
#include <fstream>
#include <stdio.h>
using namespace std;

/**
 * Primitive Brainfuck commands
 */

typedef enum { 
    INCREMENT, // +
    DECREMENT, // -
    SHIFT_LEFT, // <
    SHIFT_RIGHT, // >
    INPUT, // ,
    OUTPUT, // .
	ZERO
} Command;

// Forward references. Silly C++!
class CommandNode;
class Loop;
class Program;

/**
 * Visits?!? Well, that'd indicate visitors!
 * A visitor is an interface that allows you to walk through a tree and do stuff.
 */
class Visitor {
    public:
        virtual void visit(const CommandNode * leaf) = 0;
        virtual void visit(const Loop * loop) = 0;
        virtual void visit(const Program * program) = 0;
};

/**
 * The Node class (like a Java abstract class) accepts visitors, but since it's pure virtual, we can't use it directly.
 */
class Node {
    public:
        virtual void accept (Visitor *v) = 0;
};

/**
 * CommandNode publicly extends Node to accept visitors.
 * CommandNode represents a leaf node with a primitive Brainfuck command in it.
 */
class CommandNode : public Node {
    public:
		int count;
        Command command;
        CommandNode(char c, int num) {
			count = num;
            switch(c) {
                case '+': command = INCREMENT; break;
                case '-': command = DECREMENT; break;
                case '<': command = SHIFT_LEFT; break;
                case '>': command = SHIFT_RIGHT; break;
                case ',': command = INPUT; break;
                case '.': command = OUTPUT; break;
				case '\0': command = ZERO; break;
            }
        }
		
        void accept (Visitor * v) {
            v->visit(this);
        }
};

class Container: public Node {
    public:
        vector<Node*> children;
        virtual void accept (Visitor * v) = 0;
};

/**
 * Loop publicly extends Node to accept visitors.
 * Loop represents a loop in Brainfuck.
 */
class Loop : public Container {
    public:
        void accept (Visitor * v) {
            v->visit(this);
        }
};

/**
 * Program is the root of a Brainfuck program abstract syntax tree.
 * Because Brainfuck is so primitive, the parse tree is the abstract syntax tree.
 */
class Program : public Container {
    public:
        void accept (Visitor * v) {
            v->visit(this);
        }
};

/**
 * Read in the file by recursive descent.
 * Modify as necessary and add whatever functions you need to get things done.
 */
void parse(fstream & file, Container * container) {
    char c;
	int counter;
    Loop * loop;
	CommandNode* leaf;
	while(file.get(c)) {
		counter = 1;
		if(c == '+' || c == '-' || c == '>' || c == '<' || c == '.' || c == ',') {
			while(file.peek() == c) {
				file.get(c);
				counter++;
			}
			container->children.push_back(new CommandNode(c, counter));
		}
		else if (c == '[') {
			loop = new Loop();
			parse(file, loop);
			leaf = (CommandNode*)loop->children[0];
			// examine your loop's contents
			if(leaf->count == 1) {
				if(leaf->command == INCREMENT || leaf->command == DECREMENT) {
					container->children.push_back(new CommandNode('\0', 1));
				}
			}
			container->children.push_back(loop);
		} 
		else if (c == ']') {
			return;
		}
	}
}

/**
 * A printer for Brainfuck abstract syntax trees.
 * As a visitor, it will just print out the commands as is.
 * For Loops and the root Program node, it walks trough all the children.
 */
class Printer : public Visitor {
    public:
        void visit(const CommandNode * leaf) {
			for(int i=0; i < leaf->count; i++) {
				switch (leaf->command) {
					case INCREMENT:   cout << '+'; break;
					case DECREMENT:   cout << '-'; break;
					case SHIFT_LEFT:  cout << '<'; break;
					case SHIFT_RIGHT: cout << '>'; break;
					case INPUT:       cout << ','; break;
					case OUTPUT:      cout << '.'; break;
				}
			}
        }
        void visit(const Loop * loop) {
            cout << '[';
            for (vector<Node*>::const_iterator it = loop->children.begin(); it != loop->children.end(); ++it) {
                (*it)->accept(this);
            }
            cout << ']';
        }
        void visit(const Program * program) {
            for (vector<Node*>::const_iterator it = program->children.begin(); it != program->children.end(); ++it) {
                (*it)->accept(this);
            }
            cout << '\n';
        }
};

class Interpreter : public Visitor {
    char memory[30000];
    int pointer;
    public:
		char array[30000];
		char *ptr;
        void visit(const CommandNode * leaf) {
			for(int i=0; i<leaf->count; i++) {
				switch (leaf->command) {
					case INCREMENT:
						++*ptr;
						break;
					case DECREMENT:
						--*ptr;
						break;
					case SHIFT_LEFT:
						--ptr;
						break;
					case SHIFT_RIGHT:
						++ptr;
						break;
					case INPUT:
						*ptr = getchar();
						break;
					case OUTPUT:
						putchar(*ptr);
						break;
					case ZERO:
						*ptr = 0;
						break;
				}
			}
        }
        void visit(const Loop * loop) {
			while(*ptr) {
				for (vector<Node*>::const_iterator it = loop->children.begin(); it != loop->children.end(); ++it) {
					(*it)->accept(this);
				}
			}
        }
        void visit(const Program * program) {
			for (int i = 0; i < 30000; i++) {
				array[i] = 0;
			}
			ptr = array;
            for (vector<Node*>::const_iterator it = program->children.begin(); it != program->children.end(); ++it) {
                (*it)->accept(this);
            }
        }
};

class Compiler : public Visitor {
    public:
        void visit(const CommandNode * leaf) {
			for(int i=0; i<leaf->count; i++) {
				switch (leaf->command) {
					case INCREMENT:   cout << "++*ptr;\n"; break;
					case DECREMENT:   cout << "--*ptr;\n"; break;
					case SHIFT_LEFT:  cout << "--ptr;\n"; break;
					case SHIFT_RIGHT: cout << "++ptr;\n"; break;
					case INPUT:       cout << "*ptr=getchar();\n"; break;
					case OUTPUT:      cout << "putchar(*ptr);\n"; break;
				}
			}
        }
        void visit(const Loop * loop) {
            cout << "while (*ptr) {";
            for (vector<Node*>::const_iterator it = loop->children.begin(); it != loop->children.end(); ++it) {
                (*it)->accept(this);
            }
            cout << "}";
        }
        void visit(const Program * program) {
            cout << "#include <stdio.h>\n";
            cout << "char array[30000] = {0};\n";
            cout << "char *ptr=array;\n";
            cout << "int main(int argc, char **argv) {\n";
            for (vector<Node*>::const_iterator it = program->children.begin(); it != program->children.end(); ++it) {
                (*it)->accept(this);
            }
            cout << "}\n";
        }
};

int main(int argc, char *argv[]) {
    fstream file;
    Program program;
	Interpreter interpreter;
    Compiler printer;
    if (argc == 1) {
        cout << argv[0] << ": No input files." << endl;
    } else if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            file.open(argv[i], fstream::in);
            parse(file, & program);
            program.accept(&printer);
			cout << endl;
            program.accept(&interpreter);
			cout << endl;
            file.close();
        }
    }
}