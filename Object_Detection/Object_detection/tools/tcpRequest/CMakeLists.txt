
SET(headers_ui 
	TcpResponse.h
)

IF("${FINDOBJECT_QT_VERSION}" STREQUAL "4")
    QT4_WRAP_CPP(moc_srcs ${headers_ui})
ELSE()
    QT5_WRAP_CPP(moc_srcs ${headers_ui})
ENDIF()

SET(SRC_FILES
    TcpResponse.cpp
    main.cpp
    ${moc_srcs} 
)

SET(INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

IF("${FINDOBJECT_QT_VERSION}" STREQUAL "4")
    INCLUDE(${QT_USE_FILE})
ENDIF()


SET(LIBRARIES
	${OpenCV_LIBS} 
	${QT_LIBRARIES} 
)

# Make sure the compiler can find include files from our library.
INCLUDE_DIRECTORIES(${INCLUDE_DIRS})

# Add binary called "example" that is built from the source file "main.cpp".
# The extension is automatically found.
ADD_EXECUTABLE(tcpRequest ${SRC_FILES})
TARGET_LINK_LIBRARIES(tcpRequest find_object ${LIBRARIES})
IF("${FINDOBJECT_QT_VERSION}" STREQUAL "5")
    QT5_USE_MODULES(tcpRequest Widgets Core Gui Network PrintSupport)
ENDIF()

SET_TARGET_PROPERTIES( tcpRequest 
  PROPERTIES OUTPUT_NAME ${PROJECT_PREFIX}-tcpRequest)
  
INSTALL(TARGETS tcpRequest
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT runtime
        BUNDLE DESTINATION "${CMAKE_BUNDLE_LOCATION}" COMPONENT runtime)

