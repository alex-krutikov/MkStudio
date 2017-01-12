Product {
    type: "application"

    name: "mmpm"

    Depends { name: "cpp" }
    Depends { name: "Qt.widgets" }
    Depends { name: "Qt.network" }
    Depends { name: "modbuslib" }

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
        files: "mmpm.rc"
    }

    Group {
        fileTagsFilter: ["application"]
        qbs.install: true
    }



}

