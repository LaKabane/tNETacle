(defconst tNETacle-style
  '(
   ;; (c-auto-newline . nil)
   ;; default indentation level
   (c-basic-offset . 4)
   ;; in which column to add backslashes when macroizing a region
   (c-backslash-column . 78)
   ;; automatically compact brace-else(if)-brace on one line and
   ;; semi-colon after closing struct brace
   (c-cleanup-list . (brace-else-brace
		      brace-elseif-brace
		      defun-close-semi))
   ;; do not indent lines containing only start-of-comment more than default
   (c-comment-only-line-offset . 0)
   ;; start new lines after braces
   ;; default is: before and after (for all other cases)
   (c-hanging-braces-alist . ((defun-open . (before after))
			      (defun-close . (before after))
			      (block-open . (after))
			      (block-close . c-snug-do-while)
			      (substatement-open . after)
			      (statement-case-open . nil)
			      (brace-list-open . after)
			      (brace-list-close . nil)
			      ))
   ;; where to put newlines around colons
   (c-hanging-colons-alist . (quote ((label after)
				     (case-label after))))
   ;; indent comments syntactically
   (c-indent-comments-syntactically-p . t)
   ;; no spaces needed before a label
   ;; (c-label-minimum-indentation . 0)
   ;; define offsets for some code parts
   (c-offsets-alist . ((arglist-cont-nonempty . 2)
		       (block-open        . 0)
;;		       (block-open        . -)
		       (brace-list-entry  . 4)
		       (brace-list-open   . 4)
		       (brace-list-close  . 0)
		       (knr-argdecl       . 0)
		       (knr-argdecl-intro . +)
		       (label             . -)
		       (member-init-intro . ++)
		       (statement-cont    . 2)
		       (substatement-open . 0)
		       (case-label        . 0)))
   ;; XXX: undocumented. Recognize KNR style?
   (c-recognize-knr-p . t)
   ;; indent line when pressing tab, instead of a plain tab character
   (c-tab-always-indent . t)
   ;; use TABs for indentation, not spaces
   ;;(indent-tabs-mode . t)
   ;; set default tab width to 8
   (tab-width . 4)
  )
  "tNETacle Style")

(defun tNETacle-c-mode-hook ()
  ;; Add style and set it for current buffer
  (c-add-style "tNETacle" tNETacle-style t)
  ;; useful, but not necessary for the mode
  ;; give syntactic information in message buffer
  ;;(setq c-echo-syntactic-information-p t)
  ;; automatic newlines after special characters
  (setq c-toggle-auto-state 1)
  ;; delete all connected whitespace when pressing delete
  (setq c-toggle-hungry-state 1)
  ;; auto-indent new lines
  (define-key c-mode-base-map "\C-m" 'newline-and-indent)
)

(add-hook 'c-mode-hook 'tNETacle-c-mode-hook)

;; breaks saving -- writes "/path/to/file clean" and marks buffer dirty
;;(require 'whitespace)
;;(add-hook 'write-file-hooks 'whitespace-cleanup)
