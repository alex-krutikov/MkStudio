Product {
    type: "application"

    Depends { name: "modbuslib" }
    Depends { name: "Qt.network" }

    cpp.cxxFlags: [
          "--no-rtti",
          "--no-exceptions",
    ]

    files: [
        "mkserverd.cpp"
    ]

    Group {
        fileTagsFilter: ["application"]
        qbs.install: true
    }

}
