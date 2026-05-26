#pragma once

#if defined(_WIN32)

#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#include <windows.h>
#include <security.h>
#include <schannel.h>
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <security.h>
#include <sspi.h>
#include <schannel.h>
#include <wincrypt.h>

#define OS "Windows"

#define HAVE_WINDOWS_H 1
#define HAVE_WINSOCK2_H 1
#define HAVE_WS2TCPIP_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_FCNTL_H 1
#define HAVE_IO_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_MEMORY_H 1
#define HAVE_STDINT_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_ERRNO_H 1
#define HAVE_PROCESS_H 1
#define HAVE_SIGNAL_H 1
#define HAVE_TIME_H 1
#define HAVE_LOCALE_H 1
#define HAVE_SETLOCALE 1
#define HAVE_STRDUP 1
#define HAVE_STRICMP 1
#define HAVE_STRNICMP 1
#define HAVE_GETADDRINFO 1
#define HAVE_FREEADDRINFO 1
#define HAVE_GETNAMEINFO 1
#define HAVE_INET_NTOP 1
#define HAVE_INET_PTON 1
#define HAVE_SELECT 1
#define HAVE_IOCTLSOCKET 1
#define HAVE_CLOSESOCKET 1
#define HAVE_SOCKET 1
#define HAVE_RECV 1
#define HAVE_SEND 1

#define USE_THREADS_WIN32 1
#define USE_WINSOCK 1
#define USE_WINDOWS_SSPI 1
#define USE_SCHANNEL 1
#define USE_WIN32_IDN 1

#define CURL_DISABLE_HTTP3 1
#define CURL_DISABLE_LDAP 1
#define CURL_DISABLE_LDAPS 1
#define CURL_DISABLE_TELNET 1
#define CURL_DISABLE_DICT 1
#define CURL_DISABLE_FILE 1
#define CURL_DISABLE_TFTP 1
#define CURL_DISABLE_FTP 1
#define CURL_DISABLE_GOPHER 1
#define CURL_DISABLE_IMAP 1
#define CURL_DISABLE_POP3 1
#define CURL_DISABLE_RTSP 1
#define CURL_DISABLE_SMB 1
#define CURL_DISABLE_SMTP 1
#define CURL_DISABLE_MQTT 1

#define CURL_DISABLE_DOH 1
#define CURL_DISABLE_WEBSOCKETS 1

#define CURL_DISABLE_NEGOTIATE_AUTH 1
#define CURL_DISABLE_KERBEROS_AUTH 1
#define CURL_DISABLE_AWS 1

#define CURL_DISABLE_ALTSVC 1
#define CURL_DISABLE_HSTS 1
#define CURL_DISABLE_COOKIES 1

#define CURL_DISABLE_VERBOSE_STRINGS 1

#define SIZEOF_CURL_OFF_T 8
#define SIZEOF_INT 4
#define SIZEOF_LONG 4
#define SIZEOF_LONG_LONG 8
#define SIZEOF_OFF_T 4
#define SIZEOF_SIZE_T 8
#define SIZEOF_TIME_T 8

#define CURL_SIZEOF_CURL_OFF_T 8

#endif