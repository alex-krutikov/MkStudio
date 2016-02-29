Product {
    type: "staticlibrary"

    Depends { name: "Qt.core" }
    Depends { name: "Qt.network" }

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: ["."]
    }

    files: [
        "abstractserialport.h",
        "console.h",
        "crc.h",
        "mbcommon.h",
        "mbmaster.h",
        "mbmaster_p.h",
        "serialport.h",
        "serialport_p.h",
        "mbtcpserver.h",
        "mbtcp.h",
        "mbtypes.h",
        "packed_struct_begin.h",
        "packed_struct_end.h",
        "xfiles.h",
        "mbmaster.cpp",
        "mbcommon.cpp",
        "crc.cpp",
        "serialport.cpp",
        "console.cpp",
        "xfiles.cpp",
        "mbtcp.cpp",
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
