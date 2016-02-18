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
        "dialogs.h",
        "misc.h",
        "main.cpp",
        "mainwindow.cpp",
        "dialogs.cpp",
        "misc.cpp",
        "info.h",
        "utils.h",
        "ui/mainwindow.ui",
        "ui/assigndialog.ui",
        "ui/settingsdialog.ui",
        "ui/initdialog.ui",
        "ui/texteditdialog.ui",
        "ui/unitedslots.ui",
        "ui/shortcuts.ui",
        "mkstudio.qrc",
    ]

     Group {
        name: "Windows files"
        condition: qbs.targetOS.contains("windows")
        files: "icon.rc"
    }
}
