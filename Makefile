SUBDIRS := date_test led_test rs485_test_rcv rs485_test_snd udp_client udp_server usb_test
 
.PHONY: all
 
all:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir -j1 all || exit $$?; \
	done
clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir -j1 clean || exit $$?; \
	done