foreach(PIC scheme schemes_all) 
  add_custom_command(OUTPUT ${CURRENT_DIR}/${PIC}.eps
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${CURRENT_DIR}/${PIC}.fig
      ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_DIR}/${PIC}.fig
    COMMAND ${FIGTEX2EPS} ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_DIR}/${PIC}.fig
      ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_DIR}/${PIC}.eps
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_DIR}
    DEPENDS ${CURRENT_DIR}/${PIC}.fig)
endforeach()
