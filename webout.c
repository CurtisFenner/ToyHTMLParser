void showDocString(docstring j) {
	if (j.slen > 0) {
		for (int i = 0; i < j.slen; i++) {
			//printf("%c", *(j.str + i));
			putchar(*(j.str + i));
		}
	} else {
		int start = j.start;
		int length = j.length;
		for (int i = start; i < start + length; i++) {
			//printf("%c",(*(j.document)).text[i]);
			putchar((*(j.document)).text[i]);
		}
	}
}

void showHTML(wtag at,int tab) {
	for (int i = 0; i < tab; i++) {
		printf("\t");
	}
	wdocument doc = *(at.tagname.document);
	char* text = doc.text;
	//if (*(text + (at.tagname.start + 1)) == '#') {
	if (at.data != NULL) {
		//Show data
		showDocString(*at.data);
	} else {
		printf("<");
		showDocString(at.tagname);
		printf(">");
	}
	printf("\n");
	if (at.firstChild != NULL) {
		showHTML(*at.firstChild,tab+1);
		printf("\n");
		for (int i = 0; i < tab; i++) {
			printf("\t");
		}
		printf("</>");
		printf("\n");
	}
	if (at.nextSibling != NULL) {
		showHTML(*at.nextSibling,tab);
	}
}

void showTag(wtag t) {
	putchar('\n');
	showDocString(t.tag);
	printf(", <");
	showDocString(t.tagname);
	printf(">");
	printf("\n");
	if (t.previousSibling != NULL) {
		printf("\tPrev:\t");
		docstring g = t.previousSibling -> tag;
		showDocString(g);
		printf("\n");
	} else {
		printf("\tPrev:\tNULL\n");
	}
	if (t.nextSibling != NULL) {
		printf("\tNext:\t");
		docstring g = t.nextSibling -> tag;
		showDocString(g);
		printf("\n");
	} else {
		printf("\tNext:\tNULL\n");
	}
	if (t.parent != NULL) {
		printf("\tParent:\t");
		docstring g = t.parent -> tag;
		showDocString(g);
		printf("\n");
	} else {
		printf("\tParent:\tNULL\n");
	}
}