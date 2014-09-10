//definition of necessary web structs

typedef struct docstring docstring;
typedef struct wdocument wdocument;
typedef struct wattribute wattribute;

struct docstring {
	wdocument* document;
	long start;
	long length;
	long slen; //Non negative means use STR instead of document
	char* str;
	//This is the string value, if
	//not sourceable from document.
};

struct wattribute {
	struct docstring name;
	struct docstring value;
	wattribute* next;
};

typedef struct wtag wtag;
struct wtag {
	wtag* previousSibling;
	wtag* nextSibling;
	wtag* firstChild;
	wtag* parent;
	wattribute* firstattribute;
	docstring* data; //Text content, for such tags.
	docstring tag; //Includes <>
	docstring tagname; //Just the word (guaranteed lowercase)
};


struct lengthed {
	long size;
	char* place;
};


struct wdocument {
	wtag* root;
	char* text;
	long length;
};