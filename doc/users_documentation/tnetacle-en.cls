\NeedsTeXFormat{LaTeX2e}[1995/12/01]
\ProvidesClass{tnetacle-en}[2010/07/15 v2.0
  LaTeX class for tnetacle-themed documents - English language]

\LoadClass[en]{tnetacle}

\DeclareOption*{%
  % Pass every unknown option to the article class
  \PassOptionsToClass{\CurrentOption}{tnetacle}
}
\ProcessOptions\relax

\usepackage[english]{babel}
