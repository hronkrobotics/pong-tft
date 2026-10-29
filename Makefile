.PHONY: test firmware firmware-9486

test:
	$(MAKE) -C test

firmware:
	arduino-cli compile --warnings all --fqbn arduino:avr:nano:cpu=atmega328old firmware/Pong

firmware-9486:
	arduino-cli compile --warnings all --fqbn arduino:avr:nano:cpu=atmega328old \
		--build-property "compiler.cpp.extra_flags=-DTFT_DRIVER=9486" firmware/Pong
