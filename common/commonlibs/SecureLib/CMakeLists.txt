SET(UNIT_NAME "SecureLib")

SET( INCLUDEDIRS
${CMAKE_CURRENT_SOURCE_DIR}/..
${CLIENTLIB_EXTLIBS_DIR}/openssl/include
)

SET(${UNIT_NAME}_SRCS
VS_Certificate.cpp
VS_CertificateIssue.cpp
VS_CertMixins.cpp
VS_CryptoInit.cpp
VS_PublicKeyCrypt.cpp
VS_SecureConstants.cpp
VS_SecureHandshake.cpp
VS_Sign.cpp
VS_SSLConfigKeys.cpp
VS_StreamCrypter.cpp
VS_SymmetricCrypt.cpp
VS_UtilsLib.cpp
)
SET(${UNIT_NAME}_HDRS
SecureTypes.h
VS_Certificate.h
VS_CertificateIssue.h
VS_CertMixins.h
VS_CryptoInit.h
VS_PublicKeyCrypt.h
VS_SecureConstants.h
VS_SecureHandshake.h
VS_Sign.h
VS_SSLConfigKeys.h
VS_StreamCrypter.h
VS_SymmetricCrypt.h
VS_UtilsLib.h
)
ADD_LIBRARY(
    ${UNIT_NAME}
    STATIC
    ${${UNIT_NAME}_SRCS}
    ${${UNIT_NAME}_HDRS}
)

target_include_directories(${UNIT_NAME} PRIVATE
${INCLUDEDIRS}
${BOOST_DIR}
)
