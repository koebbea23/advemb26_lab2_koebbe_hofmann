# Unit tests for the hardware-independent logic. Runs on the Pico and in Renode.
add_executable(mytest
  test_logic.c
  unity_config.c
  ${CMAKE_SOURCE_DIR}/src/blink.c
  ${CMAKE_SOURCE_DIR}/src/case_swap.c
)

target_link_libraries(mytest PRIVATE
  pico_stdlib
  unity
)

# Modify the below lines to enable/disable output over UART/USB.
# These are mutually exclusive, only set one.
pico_enable_stdio_uart(mytest 0)
pico_enable_stdio_usb(mytest 1)
pico_add_extra_outputs(mytest)

# This will speed up the simulation, since data won't be copied from the flash controller.
# It does make the binary non-persistent on a real microcontroller.
# pico_set_binary_type(mytest no_flash)

find_program(RENODE renode
# This is extra stuff so it can find the mac installation paths.
 NAMES macos_run.command Renode.exe
 PATHS /Applications/Renode.app/Contents/MacOS)

set(RENODE_FLAGS
  --disable-xwt
  --port -2
  --pid-file renode.pid
  --console
  )

add_test(NAME simulate_mytest COMMAND
    ${RENODE}
     ${RENODE_FLAGS}
    -e "$ELF=@$<TARGET_FILE:mytest>; $WORKING=@${CMAKE_SOURCE_DIR}; include @${CMAKE_SOURCE_DIR}/simulate.resc; start"
    )


find_program(OPENOCD openocd)
find_program(PICOTOOL picotool)

add_custom_target(flash_test ${PICOTOOL} load -f $<TARGET_FILE:mytest> DEPENDS mytest)
add_custom_target(openocd_test ${OPENOCD} -s ${OPENOCD_PATH} -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c 'adapter speed 5000; program "$<TARGET_FILE:mytest>" verify reset exit' DEPENDS mytest)

# Hardware integration test: blink logic driving the real CYW43 LED.
# Pico W only; Renode does not model the CYW43.
if(PICO_CYW43_SUPPORTED)
  add_executable(blink_hwtest
    test_blink_hw.c
    unity_config.c
    ${CMAKE_SOURCE_DIR}/src/blink.c
    ${CMAKE_SOURCE_DIR}/src/led.c
  )
  target_link_libraries(blink_hwtest PRIVATE
    pico_stdlib
    pico_cyw43_arch_none
    unity
  )
  pico_enable_stdio_uart(blink_hwtest 0)
  pico_enable_stdio_usb(blink_hwtest 1)
  pico_add_extra_outputs(blink_hwtest)

  add_custom_target(flash_blink_hwtest ${PICOTOOL} load -f $<TARGET_FILE:blink_hwtest> DEPENDS blink_hwtest)
endif()
