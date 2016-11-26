Product {
    type: "application"

    Depends { name: "modbuslib" }
    Depends { name: "Qt.network" }
    Depends { name: "Qt.xml" }

    cpp.cxxFlags: [
          "--no-rtti",
          "--no-exceptions",
    ]

    files: [
        "mksync.cpp"
    ]

    Group {
        fileTagsFilter: ["application"]
        qbs.install: true
    }
}
