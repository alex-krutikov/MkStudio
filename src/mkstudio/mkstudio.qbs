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
        consoleApplication: false

        Depends { name: "qwt" }
        Depends { name: "modbuslib" }
        Depends { name: "mklib" }
        Depends { name: "Qt.widgets" }
        Depends { name: "Qt.network" }
        Depends { name: "Qt.xml" }

        cpp.cxxLanguageVersion: "c++14"

        cpp.commonCompilerFlags: [
            "-fdata-sections",
            "-ffunction-sections",
        ]

        files: mkstudioFiles

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

    Product {
        type: "application"
        name: "mkstudio-arduino"
        consoleApplication: false

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

        Group {
            fileTagsFilter: ["application"]
            qbs.install: true
        }
    }

}
