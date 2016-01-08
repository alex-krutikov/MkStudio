Product {
    type: "application"

    Depends { name: "qwt" }
    Depends { name: "modbuslib" }
    Depends { name: "mklib" }
    Depends { name: "Qt.widgets" }
    Depends { name: "Qt.network" }
    Depends { name: "Qt.xml" }

    files: [
        "main.h",
        "mainwindow.h",
        "interface.h",
        "main.cpp",
        "mainwindow.cpp",
        "ui/initdialog.ui",
        "ui/midialog.ui",
        "ui/mainwindowxml.ui",
        "mkview.qrc",
    ]

     Group {
        name: "Windows files"
        condition: qbs.targetOS.contains("windows")
        files: "icon.rc"
    }
}
