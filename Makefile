default: developers mycc

mycc: Source/mycc.c
	gcc Source/util/io.c Source/util/hash.c Source/parse/lexer.c Source/mycc.c -o mycc

developers: Documentation/developers.tex
	pdflatex -output-directory Documentation Documentation/developers.tex

.PHONY: clean
clean: 
	@echo CLEAN
	@rm -rf mycc Documentation/*.aux Documentation/*.log Documentation/*.pdf
