Project {

    property stringList mkstudioFiles: [
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

    Product {
        type: "application"
        name: "mkstudio"

        Depends { name: "qwt" }
        Depends { name: "modbuslib" }
        Depends { name: "mklib" }
        Depends { name: "Qt.widgets" }
        Depends { name: "Qt.network" }
        Depends { name: "Qt.xml" }

        files: mkstudioFiles

        Group {
            name: "Windows files"
            condition: qbs.targetOS.contains("windows")
            files: "icon.rc"
        }
    }

    Product {
        type: "application"
        name: "mkstudio-arduino"

        Depends { name: "qwt" }
        Depends { name: "modbuslib" }
        Depends { name: "mklib" }
        Depends { name: "Qt.widgets" }
        Depends { name: "Qt.network" }
        Depends { name: "Qt.xml" }

        cpp.defines: "ARDUINO_ONLY"

        files: mkstudioFiles

        Group {
            name: "Windows files"
            condition: qbs.targetOS.contains("windows")
            files: "icon.rc"
        }
    }

}
