import qbs

Project {

    Application {
        name: "baseedit"
        Depends { name: "cpp" }
        Depends { name: "Qt.widgets" }
        Depends { name: "Qt.xml" }

        cpp.cxxFlags: [
              "--no-rtti",
              "--no-exceptions",
        ]

        cpp.cxxLanguageVersion: "c++14"

        consoleApplication: false

        files: [
            "*.h",
            "*.hpp",
            "*.c",
            "*.cpp",
            "*.qrc",
            "ui/*.ui",
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
}

