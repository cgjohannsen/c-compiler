
/* Tricky cases */

struct thing {
	int x, y;
};

int main(int argc, const char* const* argv)
{
	printf("This is a C++ comment: // evil\n");
	3;
	'4';
	// Also kind of evil: /*
	not in the comment.
	/* More strange comments /* /* /* */
		*/   // <- outside the comment, so * and /
	if breakfast then eat; else starve;

	/*/ Still in a comment /*/
	out;
	/**/
	/* */
	/***/
	/***********/

	// Fun with symbols
	//
	x + y ++ z +++ what ++++ the +++++ heck ?

	charity
	re/**/char/**/ge
	avoid
	a// another split
void

	BYE

	// Bad symbols, lexer should complain

	$ `

	/*
	 *	This is an unclosed comment.
	 *	We should get an error message at the comment start.

}


