<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY dbstyle PUBLIC "-//Norman Walsh//DOCUMENT DocBook HTML Stylesheet//EN" CDATA DSSSL>
]>

<!-- parts stolen from Brian Julin's GGPG stylesheet -->

<style-sheet>
<style-specification use="docbook">
<style-specification-body>

;; Use ID attributes as name for component HTML files?
(define %use-id-as-filename% #t)

;; Default extension for generated HTML files
(define %html-ext% ".html")

;; Display comments in documentation
(element comment
	 (make element gi: "I"
	       attributes: '(("CLASS" "COMMENT"))
	       (make sequence
		     (literal "Comment: ")
		     (process-children))))

;; Some literals should be quoted so text browsers can see them.
(define ($quoted-mono-seq$)
  (make sequence
	(literal (gentext-start-quote))
	($mono-seq$)
	(literal (gentext-end-quote))))

(element literal ($quoted-mono-seq$))
(element computeroutput ($quoted-mono-seq$))

</style-specification-body>
</style-specification>
<external-specification id="docbook" document="dbstyle">
</style-sheet>
