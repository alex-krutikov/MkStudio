Product {
    type: "staticlibrary"

    Depends { name: "Qt.core" }
    Depends { name: "Qt.network" }

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: ["."]
    }

    files: [
        "mbmaster.h",
        "mbmaster_p.h",
        "serialport.h",
        "mbtcpserver.h",
        "mbmaster.cpp",
        "mbcommon.cpp",
        "crc.cpp",
        "serialport.cpp",
        "console.cpp",
        "xfiles.cpp",
        "mbtcp.cpp" ,
        "mbtcpserver.cpp",
    ]

    Group {
        name: "Windows files"
        condition: qbs.targetOS.contains("windows")
        files: "serialport_win.cpp"
    }

    Group {
        name: "Linux files"
        condition: qbs.targetOS.contains("linux")
        files: "serialport_unix.cpp"
    }

}
