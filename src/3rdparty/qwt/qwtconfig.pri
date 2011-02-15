######################################################################
# Install paths
######################################################################

VER_MAJ      = 5
VER_MIN      = 2
VER_PAT      = 1
VERSION      = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}

unix {
    INSTALLBASE    = /usr/local/qwt-$$VERSION-svn
}

win32 {
    INSTALLBASE    = C:/Qwt-$$VERSION-svn
}

target.path    = $$INSTALLBASE/lib
headers.path   = $$INSTALLBASE/include
doc.path       = $$INSTALLBASE/doc

######################################################################
# qmake internal options
######################################################################

CONFIG           += qt     # Also for Qtopia Core!
CONFIG           += warn_on
CONFIG           += thread
CONFIG           += silent

######################################################################
# If you want to have different names for the debug and release 
# versions you can add a suffix rule below.
######################################################################

DEBUG_SUFFIX        = _d
RELEASE_SUFFIX      = 

######################################################################
# Build the static/shared libraries.
# If QwtDll is enabled, a shared library is built, otherwise
# it will be a static library.
######################################################################

#CONFIG           += QwtDll

######################################################################
# QwtPlot enables all classes, that are needed to use the QwtPlot 
# widget. 
######################################################################

CONFIG       += QwtPlot

######################################################################
# QwtWidgets enables all classes, that are needed to use the all other
# widgets (sliders, dials, ...), beside QwtPlot. 
######################################################################

CONFIG     += QwtWidgets

######################################################################
# If you want to display svg imageson the plot canvas, enable the 
# line below. Note that Qwt needs the svg+xml, when enabling 
# QwtSVGItem.
######################################################################

#CONFIG     += QwtSVGItem

######################################################################
# You can use the MathML renderer of the Qt solutions package to 
# enable MathML support in Qwt.  # If you want this, copy 
# qtmmlwidget.h + qtmmlwidget.cpp to # textengines/mathml and enable 
# the line below.
######################################################################

#CONFIG     += QwtMathML

######################################################################
# If you want to build the Qwt designer plugin, 
# enable the line below.
# Otherwise you have to build it from the designer directory.
######################################################################

#CONFIG     += QwtDesigner

######################################################################
# If you want to auto build the examples, enable the line below
# Otherwise you have to build them from the examples directory.
######################################################################

#CONFIG     += QwtExamples
