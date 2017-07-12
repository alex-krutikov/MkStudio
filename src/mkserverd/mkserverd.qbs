Product {
    type: "application"

    Depends { name: "modbuslib" }
    Depends { name: "Qt.network" }

    files: [
        "mkserverd.cpp"
    ]

    Group {
        fileTagsFilter: ["application"]
        qbs.install: true
    }

}
