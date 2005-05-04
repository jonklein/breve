DOCBOOKDIR	= /sw/bin

.PHONY: classes docbook documentation source spell tidy

documentation: docclean docbook

docclean: 
	rm -rf docs

classes: docBuild/docExtract
	mkdir -p docs/classes
	docBuild/docExtract lib/classes/*.tz | docBuild/docBuild.pl docs/classes
	docBuild/indexBuild.pl lib/classes/*.tz > docs/classes/method_index.html
	cp docBuild/classImages/*.jpg docs/classes/

docbook: classes
	mkdir -p docBuild/docbook/docs
	touch docBuild/docbook/index.docbook
	cd docBuild/docbook; \
		./makeclasslist.sh ../../docs/classes > docs/classlist.docbook
	cd docBuild/docbook; \
		openjade -t sgml -V html-index -d breve.dsl breve.docbook
	cd docBuild/docbook; \
		$(DOCBOOKDIR)/collateindex.pl -g HTML.index > index.docbook
	cd docBuild/docbook; \
		openjade -t sgml -d breve.dsl breve.docbook
	cp -R docBuild/docbook/index.html docBuild/docbook/docs docs/
	mkdir docs/docs/images
	cp docBuild/docbook/images/*.jpg docs/docs/images/
	cp docBuild/docbook/images/*.gif docs/docs/images/
	cp docBuild/docbook/images/*.eps docs/docs/images/

source:
	mkdir -p ../docs/source
	doxygen breve.doxygen

spell:
	ispell -H docbook/*.docbook

tidy:
	cd docbook; tidy -config tidy.cfg -m -i -xml *.docbook
