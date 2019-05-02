PROGRAM=bms_over_mqtt
EXTRA_CFLAGS=-DLWIP_HTTPD_CGI=1 -DLWIP_HTTPD_SSI=1 -I./fsdata
EXTRA_COMPONENTS = extras/httpd extras/paho_mqtt_c extras/stdin_uart_interrupt extras/mbedtls 
#include ../../esp-open-rtos/common.mk
include ../../esp-open-rtos/myCommon.mk



#Enable debugging
EXTRA_CFLAGS+=-DLWIP_DEBUG=1 -DHTTPD_DEBUG=LWIP_DBG_ON

html:
	@echo "Generating fsdata.."
	cd fsdata && ./makefsdata
