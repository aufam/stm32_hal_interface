function(ioc_parser ioc_file name pattern output)
    file(READ ${ioc_file} ioc_content)
    string(REGEX MATCH ${pattern} match ${ioc_content})
    
    if(match)
        set(found ${CMAKE_MATCH_1})
        message(STATUS "${name}: ${found}")
        set(${output} ${found} PARENT_SCOPE)
    else()
        message(WARNING "${name} not found in ${ioc_file}")
    endif()
endfunction()

function(get_mcu_family ioc_file output)
    ioc_parser(${ioc_file} "MCU Family" "Mcu.Family=([a-zA-Z0-9]+)" result)
    set(${output} ${result} PARENT_SCOPE)
endfunction()

function(get_mcu_name ioc_file output)
    ioc_parser(${ioc_file} "MCU Name" "Mcu.Name=([a-zA-Z0-9]+)" result)
    set(${output} ${result} PARENT_SCOPE)
endfunction()

function(get_chip_part_number ioc_file output)
    ioc_parser(${ioc_file} "Chip Part Number" "Mcu.CPN=([a-zA-Z0-9]+)" result)
    set(${output} ${result} PARENT_SCOPE)
endfunction()

function(get_systick_tim_base_source ioc_file output)
    ioc_parser(${ioc_file} "TIM Base Source" "NVIC.TimeBaseIP=([a-zA-Z0-9]+)" result)
    set(${output} ${result} PARENT_SCOPE)
endfunction()

function(get_can_fifo_rx_interrupt ioc_file output)
    ioc_parser(${ioc_file} "CAN RX IRQn" "NVIC\\.CAN([0-9]*)[_\\w]*_RX([01])_IRQn=true" result)
    set(${output} ${result} PARENT_SCOPE)
endfunction()

