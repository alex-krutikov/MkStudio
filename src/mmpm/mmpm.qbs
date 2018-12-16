Product {
    type: "application"

    name: "mmpm"

    Depends { name: "cpp" }
    Depends { name: "Qt.widgets" }
    Depends { name: "Qt.network" }
    Depends { name: "Qt.xml" }
    Depends { name: "modbuslib" }

    cpp.cxxLanguageVersion: "c++14"

    Properties {
        condition: cpp.compilerName.contains("cl.exe") // MSVC compiler
        cpp.cxxFlags: "/utf-8"
    }

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

