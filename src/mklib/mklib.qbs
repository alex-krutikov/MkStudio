﻿Product {
    type: "staticlibrary"

    Depends { name: "qwt" }
    Depends { name: "modbuslib" }
    Depends { name: "Qt.widgets" }
    Depends { name: "Qt.xml" }

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: ["."]
        cpp.cxxLanguageVersion: "c++14"
    }

    cpp.cxxLanguageVersion: "c++14"

    Properties {
        condition: cpp.compilerName.contains("cl.exe") // MSVC compiler
        cpp.cxxFlags: "/utf-8"
    }

    cpp.includePaths: ["."]

    files: [
        "mk_global.h",
        "mbconfigwidget.h",
        "mbmasterwidget.h",
        "consolewidget.h",
        "mbmasterxml.h",
        "mktable.h",
        "mktable_p.h",
        "plot.h",
        "helpwidget.h",
        "slotwidget.h",
        "slotconfigtableview.h",
        "scaledraw.h",
        "mkpicker.h",
        "shortcut.h",
        "mbconfigwidget.cpp",
        "mbmasterwidget.cpp",
        "consolewidget.cpp",
        "mbmasterxml.cpp",
        "mktable.cpp",
        "plot.cpp",
        "helpwidget.cpp",
        "slotwidget.cpp",
        "slotconfigtableview.cpp",
        "mkpicker.cpp",
        "ui/mbconfigwidget.ui",
        "ui/mbmasterwidget.ui",
        "ui/mbmslotexportdialog.ui",
        "ui/plot.ui",
        "ui/consolewidget.ui",
        "ui/helpwidget.ui",
        "ui/slotwidget.ui",
        "ui/slotdialog.ui",
        "ui/plotisrdialog.ui",
        "mklib.qrc",
    ]

}
