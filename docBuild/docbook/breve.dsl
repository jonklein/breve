<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY dbstyle SYSTEM "/opt/fink/share/sgml/dsssl/docbook-dsssl-nwalsh/html/docbook.dsl" CDATA DSSSL>
<!ENTITY nav-prev        "[previous]">

]>

<style-sheet>
<style-specification use="docbook">
<style-specification-body>

(define %chapter-autolabel% #t)
(define %section-autolabel% #t)

(define %admon-graphics% #t)
(define %shade-verbatim% #t)
(define %admon-graphics-path% "docs/images/")
(define %html-prefix% "docs/")
(define (toc-depth nd) 2)

(define ($generate-chapter-toc$) #f)

(define %root-filename% "index")    
(define %html-ext% ".html")     
(define %use-id-as-filename% #t)        

(define %linenumber-mod% 1)
(define %number-programlisting-lines% 't)

(define %html-pubid% "-//W3C//DTD HTML 4.01 Transitional//EN")
(define %html40% #t)
(define %css-decoration% #t)

(define preferred-mediaobject-extensions  (list "png" "jpg" "jpeg"))        
(define preferred-mediaobject-notations (list "PNG" "JPG" "JPEG"))

;; add style sheet info to the document

(define ($user-html-header$ #!optional
                            (home (empty-node-list))
                            (up (empty-node-list))
                            (prev (empty-node-list))
                            (next (empty-node-list)))
  (make sequence
    (make element gi: "STYLE"
         attributes: (list (list "TYPE" "text/css"))
         (literal "BODY, P, LI {
	color: black;
	font-family: Verdana, Arial, sans-serif;
	font-size: small;
}

BODY {
	margin-left: 5%;
	margin-right: 5%;
}

DIV.MEDIAOBJECT {
    text-align: center;
}

PRE.SCREEN, PRE.PROGRAMLISTING {
	font-size: 80%;
}

.SECT2 P, .SECT2 UL {
        margin-left: 2%;
}

H2.SECT2 { 
        margin-left: 1%;
}

.SECT3 P, .SECT2 UL {
        margin-left: 4%;
}

H3.SECT3 {
        margin-left: 3%;
}

TABLE.SCREEN, TABLE.PROGRAMLISTING {
	margin-top: .7em;
	margin-bottom: .7em;
	margin-left: 10%;
	margin-right: 10%;
}

.NAVFOOTER TABLE, .NAVHEADER TABLE {
	margin-left: 0%;
	font-size: 70%;
}
        ")
      )
	)
)

;; add the CLASS tag to the verbatim tags

(define ($shade-verbatim-attr$)
  (list
   (list "BORDER" "0")
   (list "BGCOLOR" "#E0E0E0")
   (list "CLASS" (gi))
   (list "WIDTH" ($table-width$))))

;; change the table width of verbatim tags

(define ($table-width$)
  (if 
	(or (has-ancestor-member? (current-node) '("LISTITEM"))
     (or (has-ancestor-member? (current-node) '("SCREEN"))
	 (has-ancestor-member? (current-node) '("PROGRAMLISTING"))))
      "80%"
      "100%"))

;; change the navigation text

(define (gentext-en-nav-prev prev)
	(make sequence (literal "<< previous")))

(define (gentext-en-nav-next next)
	(make sequence (literal "next >>")))

(define (gentext-en-nav-home next)
	(make sequence (literal "breve Documentation table of contents")))

(define (gentext-en-nav-up next)
	(make sequence (literal "up")))

</style-specification-body>
</style-specification>
<external-specification id="docbook" document="dbstyle">
</style-sheet>
