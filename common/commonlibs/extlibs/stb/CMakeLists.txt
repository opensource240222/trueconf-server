SET(UNIT_NAME "stb")

SET(${UNIT_NAME}_SRCS
../../vs.natvis
stb_image.c
stb_truetype.c
)
SET(${UNIT_NAME}_HDRS
stb_image.h
stb_truetype.h
)
ADD_LIBRARY(
    ${UNIT_NAME}
    STATIC
    ${${UNIT_NAME}_SRCS}
    ${${UNIT_NAME}_HDRS}
)
