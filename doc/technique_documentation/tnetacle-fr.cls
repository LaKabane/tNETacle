\NeedsTeXFormat{LaTeX2e}[1995/12/01]
\ProvidesClass{tnetacle-fr}[2010/07/15 v2.0
  LaTeX class for tnetacle-themed documents - French language]

\LoadClass[fr]{tnetacle}

\DeclareOption*{%
  % Pass every unknown option to the article class
  \PassOptionsToClass{\CurrentOption}{tnetacle}
}
\ProcessOptions\relax

\usepackage[french]{babel}
