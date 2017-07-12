Product {
    type: "application"

    Depends { name: "modbuslib" }
    Depends { name: "Qt.network" }

    files: [
        "mkquery.cpp"
    ]

    Group {
        fileTagsFilter: ["application"]
        qbs.install: true
    }
}
