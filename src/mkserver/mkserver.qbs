Product {
    type: "application"

    Depends { name: "modbuslib" }
    Depends { name: "Qt.widgets" }
    Depends { name: "Qt.network" }
    Depends { name: "Qt.xml" }

    cpp.cxxLanguageVersion: "c++14"

    consoleApplication: false

    files: [
        "mainwindow.h",
        "main.h",
        "mainwindow.cpp",
        "main.cpp",
        "ui/mainwindow.ui",
        "mkserver.qrc",
    ]

    Group {
        name: "Windows files"
        condition: qbs.targetOS.contains("windows")
        files: "icon.rc"
    }

    Group {
        fileTagsFilter: ["application"]
        qbs.install: true
    }
}
