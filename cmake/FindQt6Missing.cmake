# Stub CMake configs for missing Qt6 modules
# These modules aren't installed in the Qt 6.10.2 gcc_64 desktop build
# but SavvyCAN's sources reference them. We provide empty targets.

function(_stub_qt_module MODNAME)
    if(NOT TARGET Qt6::${MODNAME})
        add_library(Qt6::${MODNAME} INTERFACE IMPORTED)
        set_target_properties(Qt6::${MODNAME} PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ""
        )
    endif()
    set(Qt6${MODNAME}_FOUND TRUE CACHE BOOL "" FORCE)
endfunction()

_stub_qt_module(SerialBus)
_stub_qt_module(SerialPort)
_stub_qt_module(HttpServer)
_stub_qt_module(WebSockets)
