Product {
    type: "application"

    Depends { name: "modbuslib" }
    Depends { name: "Qt.network" }
    Depends { name: "Qt.xml" }

    files: [
        "mksync.cpp"
    ]
}
