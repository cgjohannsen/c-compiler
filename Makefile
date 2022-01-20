default: developers mycc

mycc: Source/mycc.c
	gcc Source/mycc.c -o mycc

developers: Documentation/developers.tex
	pdflatex -output-directory Documentation Documentation/developers.tex

.PHONY: clean
clean: 
	rm mycc Documentation/*.aux Documentation/*.log Documentation/*.pdf
