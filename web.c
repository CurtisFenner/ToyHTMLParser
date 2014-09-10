#include <stdio.h>
#include <windows.h>
#include <stdbool.h>


#include "webstructs.c"
#include "webout.c"
#include "wrender.c"

struct lengthed fileIntoString(FILE *file) {
	fseek(file, 0, SEEK_END);
	long size;
	size = ftell(file);
	rewind(file);
	char *str = malloc((size+1) * (sizeof(char)));
	fread(str, sizeof(char), size, file);
	str[size] = '\0';
	fclose(file);
	struct lengthed returnable = {};
	returnable.size = size;
	returnable.place = str;
	return returnable;
}

//Compare must be a lowercase string
//The case of wtag's tagname is insensitive.
bool tagIs(wtag t,char* compare,int len) {
	//Compare t.tagname to compare[0...len-1]
	docstring tn = t.tagname;
	if (tn.slen > 0) {
		return false;
	} else {
		if (tn.length > len + 2) {
			return false;
		}
		for (int i = 0; i < len; i++) {
			char left = *(tn.document -> text + i + tn.start);
			char right = *(compare + i);
			if (left >= 'A' && left <= 'Z') {
				left -= ('A' - 'a');
			}
			if (left != right) {
				return false;
			}
		}
		return true;
	}
}



bool isSelfClosing(wtag s) {
	docstring d = s.tagname;
	//Takes a docstring of the tag NAME not the tag itself.
	char first = d.document -> text[d.start+0];
	if (tagIs(s,"area",4) || tagIs(s,"base",4) || tagIs(s,"br",2) || tagIs(s,"col",3) || tagIs(s,"embed",5)) {
		return true;
	}
	if (tagIs(s,"hr",2) || tagIs(s,"img",3) || tagIs(s,"input",5) || tagIs(s,"link",4) || tagIs(s,"meta",4) || tagIs(s,"param",5) || tagIs(s,"source",6)) {
		return true;
	}
	return first == '!';
}

wdocument* html_document(FILE* infile) {
	char *file;
	struct lengthed from = fileIntoString(infile);
	file = from.place;
	for (int i = 0; i < from.size; i++) {
		char c = file[i];
		if (c == '\t' || c == '\n' || c == '\r') {
			file[i] = ' ';
		}
	}
	wdocument *document = malloc(sizeof(wdocument));
	document -> text = file;
	document -> length = from.size;

	wtag* root = malloc(sizeof(wtag));
	root -> parent = NULL;
	root -> nextSibling = NULL;
	root -> firstChild = NULL;
	root -> previousSibling = NULL;

	docstring rootname;
	rootname.slen = 9;
	rootname.str = &("#document");
	rootname.document = document;
	rootname.start = 0;
	rootname.length = 0;

	root -> tagname = rootname;
	root -> tag = rootname;
	root -> data = &rootname;

	document -> root = root;

	const int WAIT_FOR_OPEN_TAG = 0; //Waits for a tag to start (waiting for <)
	const int WAIT_FOR_NAME_END = 1; //Waits for a tag's name to end either at a space or >
	const int WAIT_FOR_ATTRIBUTE = 8;
	const int WAIT_FOR_ATTRIBUTE_END = 2; //either =, space, or > to mark end of attribute name.
	const int WAIT_FOR_TAG_END = 3; //Waits for either > or a letter.

	const int WAIT_FOR_SCRIPT_CLOSE = 7; //Waits for the string </script> to declare a script tag closed.
	const int WAIT_FOR_ATTRIBUTE_VALUE = 9;
	const int WAIT_FOR_ATTRIBUTE_QUOTING = 10;
	const int WAIT_FOR_ATTRIBUTE_VALUE_END_SPACE = 11;
	const int WAIT_FOR_ATTRIBUTE_VALUE_END_DOUBLE = 12;
	const int WAIT_FOR_ATTRIBUTE_VALUE_END_SINGLE = 13;
	const int CLOSE_PARENT = -1; //Close the parent tag
	const int OPEN_NEW = -2; //Close the previous tag
	const int DO_NOTHING = -3; //Do nothing (e.g., </p> if i was being
		//really compliant and liked to do this for some reason)
	const int BE_SIBLING = -4; // E.g., <br><br><br>..

	docstring TEXTNODE;

	TEXTNODE.document = document;
	TEXTNODE.slen = 5;
	TEXTNODE.str = &("#text");

	int within[20];
	for (int i = 0; i < 20; i++) {
		within[i] = 0; //Debugging only
	}

	wtag* previous = NULL; //The previous sibling.
	wtag* parent = root;
	wtag* now = NULL;
	wattribute* attr = NULL;
	wattribute* pattr = NULL;
	//The previous attribute on the current node.
	//To be reset upon creating a new `now`

	int nmode = 0;

	for (int i = 0; i < document -> length; i++) {
		char c = file[i];
		int mode = nmode;
		within[mode]++;
		if (mode == WAIT_FOR_OPEN_TAG) {
			//Waits for a tag to start (waiting for <)
			if (c == '<') {
				if (now != NULL) {
					//A text node that we have to append in.
					if (previous == NULL) {
						parent -> firstChild = now;
						previous = now;
					} else {
						previous -> nextSibling = now;
						previous = now;
					}
				}
				now = malloc(sizeof(wtag));
				now -> parent = NULL;
				now -> firstChild = NULL;
				now -> previousSibling = NULL;
				now -> nextSibling = NULL;
				now -> data = NULL;
				now -> tag.slen = 0;
				now -> tag.document = document;
				now -> tag.start = i;
				now -> tag.length = 0;
				now -> tagname.slen = 0;
				now -> tagname.document = document;
				now -> tagname.start = i + 1;
				now -> tagname.length = 0;
				nmode = WAIT_FOR_NAME_END;
			} else {
				if (now != NULL) {
					//We don't really have anything to do, except perhaps update the length.
					now -> data -> length = i + 1 - now -> data -> start;//(*(*now).data).start;
				} else {
					now = malloc(sizeof(wtag));
					now -> parent = parent;
					now -> previousSibling = previous;
					now -> nextSibling = NULL;
					now -> firstChild = NULL;
					now -> tag = TEXTNODE;
					now -> tagname = TEXTNODE;
					now -> data = malloc(sizeof(docstring));
					now -> data -> slen = 0;
					now -> data -> document = document;
					now -> data -> start = i;
					pattr = NULL;
					attr = NULL;
				}
			}
		}
		if (mode == WAIT_FOR_ATTRIBUTE_END) {
			if (c == ' ') {
				//The attribute's name is over.
				//We now have to wait for:
				//a) = sign (write attribute value)
				//b) letter (start new tag, set up this one to copy value from name)
				//c) > (finish tag)
				attr -> name.length = i - attr -> name.start;
				nmode = WAIT_FOR_ATTRIBUTE_VALUE;
			}
			if (c == '=') {
				//Wait for attribute value.
				attr -> name.length = i - attr -> name.start;
				mode = WAIT_FOR_ATTRIBUTE_VALUE;
			}
			if (c == '>') {
				mode = WAIT_FOR_NAME_END;
			}
		}
		if (mode == WAIT_FOR_ATTRIBUTE_VALUE_END_DOUBLE || mode == WAIT_FOR_ATTRIBUTE_VALUE_END_SINGLE) {
			char match = '"';
			if (mode == WAIT_FOR_ATTRIBUTE_VALUE_END_SINGLE) {
				match = '\'';
			}
			if (file[i-1] != '\\' && c == match) {
				//The attribute is over.
				attr -> value.length = i - (attr -> value.start);
				pattr = attr;
				nmode = WAIT_FOR_ATTRIBUTE;
			}
		}
		if (mode == WAIT_FOR_ATTRIBUTE_VALUE_END_SPACE) {
			if (c == ' ' || c == '>') {
				attr -> value.length = i - (attr -> value.start);
				pattr = attr;
				mode = WAIT_FOR_ATTRIBUTE;
				nmode = WAIT_FOR_ATTRIBUTE;
			}
		}
		if (mode == WAIT_FOR_ATTRIBUTE_QUOTING) {
			if (c == '"') {
				attr -> value.start = i + 1;
				nmode = WAIT_FOR_ATTRIBUTE_VALUE_END_DOUBLE;
			}
			if (c == '\'') {
				attr -> value.start = i + 1;
				nmode = WAIT_FOR_ATTRIBUTE_VALUE_END_SINGLE;
			}
			bool lower = c >= 'a' && c <= 'z';
			bool upper = c >= 'A' && c <= 'Z';
			bool number = c >= '0' && c <= '9';
			if (lower || upper || number) {
				attr -> value.start = i;
				nmode = WAIT_FOR_ATTRIBUTE_VALUE_END_SPACE;
			}
		}
		if (mode == WAIT_FOR_ATTRIBUTE_VALUE) {
			if (c == '=') {
				//We have a specified value
				nmode = WAIT_FOR_ATTRIBUTE_QUOTING;
			}
			if (c >= 'a' && c <= 'z') {
				//We have not hit an =, so this is a new attribute.
				//Specify value to be the same as the name
				attr -> value = attr -> name;
				mode = WAIT_FOR_ATTRIBUTE; //Process accordingly
				pattr = attr;
			}
		}
		if (mode == WAIT_FOR_ATTRIBUTE) {
			if (c >= 'a' && c <= 'z') {
				attr = malloc(sizeof(wattribute));
				if (pattr != NULL) {
					pattr -> next = attr;
				}
				attr -> name.document = document;
				attr -> value.document = document;
				attr -> next = NULL;
				if (now -> firstattribute == NULL) {
					now -> firstattribute = attr;
				}
				nmode = WAIT_FOR_ATTRIBUTE_END;
			}
			if (c == '>') {
				mode = WAIT_FOR_NAME_END; //Finish up the tag.
			}
		}
		if (mode == WAIT_FOR_NAME_END) {
			//Waits for a tag's name to end either at a space or >
			if (c == ' ') {
				//Attributes TODO:
				now -> tagname.length = i - now -> tagname.start;
				nmode = WAIT_FOR_ATTRIBUTE;
			}
			if (c == '>') {
				//The tag is over.
				now -> tag.length = i - now -> tag.start + 1; //Includes < and >
				if (now -> tagname.length == 0) {
					now -> tagname.length = i - now -> tagname.start; //Includes < and >
				}

				char firstLetter = document -> text[ (now -> tag).start + 1 ];
				int action = OPEN_NEW;
				if (firstLetter == '/') {
					action = CLOSE_PARENT;
				}
				if (isSelfClosing(*now)) {
					action = BE_SIBLING;
				}
				if (action == BE_SIBLING || action == OPEN_NEW) {
					//Self closing + open tags
					//Extra closey exemptions:
					bool closeParent = false;
					if (tagIs(*parent,"head",4)) {
						if (tagIs(*now,"body",4)) {
							closeParent = true;
						}
					}
					if (tagIs(*parent,"p",1)) {
						if (tagIs(*now,"p",1)) {
							closeParent = true;
						}
						//address,article,aside,blockquote
						//div,dl,fieldset,footer,form,h1
						//h2,h3,h4,h5,h6,header,hr,menu
						//nav,ol,pre,section,table,ul,p
						if (tagIs(*now,"address",7) || tagIs(*now,"article",7) || tagIs(*now,"aside",5) || tagIs(*now,"blockquote",10)) {
							closeParent = true;
						}
						if (tagIs(*now,"div",3) || tagIs(*now,"dl",2) || tagIs(*now,"fieldset",8) || tagIs(*now,"footer",6) || tagIs(*now,"form",4) || tagIs(*now,"h1",2)) {
							closeParent = true;
						}
						if (tagIs(*now,"h2",2) || tagIs(*now,"h3",2) || tagIs(*now,"h4",2) || tagIs(*now,"h5",2) || tagIs(*now,"header",6) || tagIs(*now,"hr",2) || tagIs(*now,"menu",4)) {
							closeParent = true;
						}
						if (tagIs(*now,"nav",3) || tagIs(*now,"ol",2) || tagIs(*now,"pre",3) || tagIs(*now,"section",7) || tagIs(*now,"table",5) || tagIs(*now,"ul",2)) {
							closeParent = true;
						}
					}
					if (tagIs(*parent,"li",2)) {
						//The parent is an LI tag. The following will close it:
						//LI
						if (tagIs(*now,"li",2)) {
							closeParent = true;
						}
					}
					if (closeParent) {
							//We don't actually want the parent.
							//We make these swaps:
							//		previous <-- parent
							//		parent <-- parent.parent
							//Then we attach like normal.
							previous = parent;
							parent = previous -> parent;
					}
					if (parent != NULL && parent -> firstChild == NULL) {
						parent -> firstChild = now;
					} else {
						if (previous == NULL ) {
						}
						previous -> nextSibling = now;
						now -> previousSibling = previous;
					}
					now -> parent = parent;
					previous = now;
				}
				if (action == OPEN_NEW) {
					//Only open tags
					previous = NULL;
					parent = now;
				}
				if (action == CLOSE_PARENT) {
					//Closing tags
					free(now);
					now = NULL;
					previous = parent;
					parent = parent -> parent;
				}
				now = NULL;
				nmode = WAIT_FOR_OPEN_TAG;
			}
		}
		//showDocString(*parent -> tag);
	}
	if (now != NULL && previous != NULL) {
		//Dangling text node
		previous -> nextSibling = now;
	}

	return document;
}

const int FOREGROUND_WHITE = FOREGROUND_INTENSITY+FOREGROUND_BLUE+FOREGROUND_GREEN+FOREGROUND_RED;

int main() {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	WORD saved_attributes;
	/* Save current attributes */
	GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
	SetConsoleTextAttribute(hConsole,FOREGROUND_INTENSITY+FOREGROUND_BLUE+FOREGROUND_GREEN+FOREGROUND_RED);
	saved_attributes = consoleInfo.wAttributes;

	SetConsoleTextAttribute(hConsole, FOREGROUND_RED+FOREGROUND_INTENSITY);
	printf("PARSING INPUT HTML FILE \"SIMPLE.HTML\"\n");
	SetConsoleTextAttribute(hConsole,FOREGROUND_WHITE);

	/* Restore original attributes */

	wdocument doc = *html_document(fopen("simple.html","rb"));
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED+FOREGROUND_INTENSITY);
	printf("DISPLAYING TREE");
	SetConsoleTextAttribute(hConsole,FOREGROUND_WHITE);
	SetConsoleTextAttribute(hConsole, saved_attributes);
	wtag root= *doc.root;
	wtag child = *(root.firstChild);
	SetConsoleTextAttribute(hConsole, saved_attributes);
	printf("\n\n\n\n\n");
	showHTML(child,0);
	printf("\n\n\n\n\n");


	for (int i = 0; i < 255; i++) {
		if (i % 17 == 0) {
			putchar('\n');
		}
		if (i % 2 == 0) {
			SetConsoleTextAttribute(hConsole, BACKGROUND_RED+FOREGROUND_INTENSITY);
		} else {
			SetConsoleTextAttribute(hConsole, BACKGROUND_BLUE+FOREGROUND_INTENSITY);

		}
		if (i != '\r' && i != '\n') {
			putchar(i);
		} else {
			putchar(' ');
		}
	}
	return 0;
}