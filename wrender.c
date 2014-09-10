//wrender.c

typedef struct wrow wrow;
typedef struct wbox wbox;

const int D_BLOCK = 0;
const int D_LEFT = 1;
const int D_RIGHT = 2;
const int D_INLINE = 3;

const int A_LEFT = 0;
const int A_RIGHT = 1;
const int A_CENTER = 2;
const int A_JUSTIFY = 3;
const int V_TOP = 0;
const int V_CENTER = 1;
const int V_BOTTOM = 2;

const int F_LEFT = 0;
const int F_RIGHT = 1;
const int F_NONE = 3;

struct wbox {
	int display;
	/*
	block:Breaks follow it.
	inline: Breaks do not follow it unless the following content is wider than initial space.
	*/
	int width; //The width computed in pixels for the element.
	//An "auto" width is assigned to block elements that are not floated
	//An inline element is actually here treated as inline block (does not break at spaces).
	//It has its width specified to just fit its text.
	//Block elements ignore the space taken up by floating elements.
	//Nonetheless, children inline elements respect the space reserved by floating elements.
	//Inline elements stay within the space unused by floating elements (respecting their margins, too)
}

struct wrow {
	//A list of wbox's forming a row of inline text.

}

/*
Display analysis:

Float
	Float left and float right will not overlap. They break and continue rendering.
	"auto" width is not full for floating elements, only to fit content, though they are block.
	Float elements reserve the space of their box in addition to their margins.
		This box affects all inline elements (occurring later) despite their position in the hierarchy.
	In document order, orders are placed in the same line, building from left for left and right for right.
		Document order for floating elements. Non floating elements take remaining space.
	If there is not space, it breaks onto the next line as close vertically as space is not allocated.

Block
	"auto" default fills the space so that width + margins + padding + border + outline is the parent available width (ignoring floating elements)
	Ignores floating elements, but children do not
		Even if the first line is unacceptable for inline text, the top of the box remains there and breaks are introduced until it is acceptable.
		This gap is NOT based on lines but it as EARLY as there is space for.

*/