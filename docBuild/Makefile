DOCBOOKDIR	= /sw/bin

.PHONY: classes docbook documentation source spell tidy

documentation: docclean docbook

docclean: 
	rm -rf docs
	mkdir docs

docBuild/docExtract: docBuild/docExtract.c
	# g++ chokes here?!

	$(CC) -o docBuild/docExtract docBuild/docExtract.c

classes: docBuild/docExtract
	mkdir -p docs/classes
	docBuild/docExtract lib/classes/*.tz | docBuild/docBuild.pl docs/classes
	cp docBuild/classImages/*.jpg docs/classes/

imagecopy:
	cp docBuild/docbook/images/breve_icon.jpg docs/
	mkdir -p docs/docs/images
	cp docBuild/docbook/images/*.jpg docs/docs/images/
	cp docBuild/docbook/images/*.gif docs/docs/images/
	cp docBuild/docbook/images/*.eps docs/docs/images/

docbook-pdf: 
	cd docBuild/docbook; xsltproc --output breve_documentation.fo --stringparam fop.extensions 1 /usr/local/lib/docbook-xsl-1.70.1/fo/docbook.xsl breve.docbook
	cd docBuild/docbook; fop -fo breve_documentation.fo -pdf breve_documentation.pdf

docbook: imagecopy classes

	echo "$(VERSION)" > docBuild/docbook/version.txt
	date +'%m-%d-%Y' > docBuild/docbook/date.txt

	mkdir -p docBuild/docbook/docs

	touch docBuild/docbook/index.docbook

	cd docBuild/docbook; \
		./makeclasslist.sh ../../docs/classes > classlist.docbook

	cd docBuild/docbook; \
		xsltproc make-html.xsl breve.docbook

	cp -R docBuild/docbook/index.html docBuild/docbook/docs docs/


source:
	mkdir -p ../docs/source
	doxygen breve.doxygen

spell:
	ispell -H docbook/*.docbook

tidy:
	cd docbook; tidy -config tidy.cfg -m -i -xml *.docbook
