\NeedsTeXFormat{LaTeX2e}[1995/12/01]
\ProvidesClass{tnetacle}[2010/07/15 v2.0
  LaTeX class for tnetacle-themed documents]

\LoadClass[a4paper,12pt,onecolumn, final]{article}

\DeclareOption*{%
  % Pass every unknown option to the article class
  \PassOptionsToClass{\CurrentOption}{article}
}
\ProcessOptions\relax

% preinclude lots of stuff
%\usepackage[top=1in,bottom=1in,left=1in,right=1in]{geometry}
\usepackage[pdftex]{graphicx}
\usepackage{url}
\usepackage{fancyhdr}
\usepackage{array}
\usepackage[table]{xcolor}
\usepackage[utf8]{inputenc}
\usepackage[T1]{fontenc}
\usepackage{lmodern}
\usepackage{listings}
\usepackage{color}
\usepackage[pdftex]{hyperref}
\usepackage{fancybox}
\usepackage{float}
\usepackage{tocloft}
\usepackage{textcomp}
\usepackage{verbatim}
\usepackage{subfig}

% custom color
\definecolor{lightblack}{RGB}{20,20,20}

% custom urls
\hypersetup
{
  colorlinks,
%  citecolor=black,                %
%  filecolor=black,                %
  linkcolor=black,                % black links
  urlcolor=blue                   % blue URLs
}

% standard listing style
\lstset
{
	language=C,
	basicstyle=\ttfamily\color{white}\footnotesize,
        extendedchars=true,
	breaklines=true,
	columns=fullflexible,
	backgroundcolor=\color{lightblack},
	xleftmargin=17pt,
	framexleftmargin=17pt,
	showstringspaces=false,
        framexrightmargin=5pt,
	framexbottommargin=4pt,
	frame=single
}

% custom tnetacle background that depends of back.png
\fancyput(-2.54cm,-27.16cm){
\includegraphics[width=21cm,height=29.7cm]{back-tnetacle-eip-bw.jpeg}
}

% Using  roman  numerals  makes  the  TOC  section/subsection  numbers
% overflow on the  right. This adds some margin to  avoid that kind of
% problems. (requires \usepackage{tocloft})
\setlength{\cftsecnumwidth}{1cm}
\setlength{\cftsubsecnumwidth}{1.3cm}
\setlength{\cftsubsubsecnumwidth}{1.6cm}

\hyphenation{Mich-aux Thoul-ouze}
\makeatletter

\edef\@authors{}

\newcommand {\member}[2]
{
  \protected@edef\@authors
  {
    \@authors #1 \protect\url{#2} \\
  }
  \author
  {
    \@authors
  }
}
\author{}

\newcommand {\subtitle}[1]
{
  \def \@subtitle {#1}
}

\newcommand {\uptitle}[1]
{
  \def \@uptitle {#1}
}

\newcommand {\summary}[1]
{
  \def \@summary {#1}
}

\def \maketitle
{
  \begin{center}
	\begin{figure}
	\subfloat{\includegraphics[width=0.7\textwidth]{logo_tNETacle}}
	\subfloat{\includegraphics[width=0.3\textwidth]{logo_eip}}
	\end{figure}
  	{
	  \Huge\bfseries
	  \@title
	}
        {
          \vfill
	  \@subtitle
        }
	{
	  \vfill
	  \@summary
	}
	{
	  \vfill
	  \par
	  \large\texttt{(|\texteuro\textasciitilde}
	}
  \end{center}
  \vfill
  \@author
  \newpage
}

\renewcommand{\section}
{
  \newpage
  \@startsection{section}
                {1}
                {\z@}
                {-3.25ex \@plus -1ex \@minus -.2ex}
                {1.5ex \@plus .2ex}
                {\normalfont\LARGE\bfseries}
}

\renewcommand{\subsection}
{
  \@startsection{subsection}
                {2}
                {\z@}
                {-3.25ex \@plus -1ex \@minus -.2ex}
                {1.5ex \@plus .2ex}
                {\underline\normalfont\large\bfseries\underline}
}

\renewcommand{\subsubsection}
{
  \@startsection{subsubsection}
                {3}
                {\z@}
                {-3.25ex \@plus -1ex \@minus -.2ex}
                {1.5ex \@plus .2ex}
                {\normalfont\normalsize\bfseries\it\underline}
}
\makeatother
